
internal ShaderProgram NullShader
= {};


internal inline GLuint LoadShader(GLenum Type,MemoryMappedFile ShaderFile, const
			 char * FileName)
{
    GLuint ShaderID = glCreateShader(Type);
    GLint length=(int)ShaderFile.FileSize;
    glShaderSource(ShaderID,1,(const char **)&ShaderFile.MMapBuffer,&length);
    glCompileShader(ShaderID);
    GLint ShaderSuccess;
    glGetShaderiv(ShaderID,GL_COMPILE_STATUS,&ShaderSuccess);
    if(ShaderSuccess != GL_TRUE)
    {
	const int MAX_SHADER_LOG_LENGTH = 1024;
	int LogLen;
	glGetShaderiv(ShaderID,GL_INFO_LOG_LENGTH,&LogLen);
	//GLchar * ShaderLog = (GLchar *)alloca(LogLen);
	GLchar ShaderLog[MAX_SHADER_LOG_LENGTH];
	Assert(LogLen<MAX_SHADER_LOG_LENGTH);
	glGetShaderInfoLog(ShaderID,MAX_SHADER_LOG_LENGTH,NULL,ShaderLog);
	LogError("Compile error while compiling shader %s:",FileName);
	LogDebug("%s",ShaderLog);
	return 0;
    }
    return ShaderID;
}

internal void UnloadShaderProgram(ShaderProgram * Program)
{
    if(Program->ProgramID)
    {
	if(Program->VertexID)
	{
	    glDetachShader(Program->ProgramID,Program->VertexID);
	    glDeleteShader(Program->VertexID);
	}
	if(Program->PixelID)
	{
	    glDetachShader(Program->ProgramID,Program->PixelID);
	    glDeleteShader(Program->PixelID);
	}
	glDeleteProgram(Program->ProgramID);
    }
}

internal void UnloadAllShaders(ShaderGroup * ShaderGroup)
{
    for(int i=0;i<ShaderGroup->NumberOfShaders;i++)
    {
	UnloadShaderProgram(&ShaderGroup->Shaders[i]);
    }
    ShaderGroup->NumberOfShaders=0;
}

internal ShaderProgram * LoadShaderProgram( const char * VertexShaderFileName, const char * PixelShaderFileName, ShaderGroup * ShaderGroup)
{
    ShaderProgram * Shader = &ShaderGroup->Shaders[ShaderGroup->NumberOfShaders++];
    *Shader = {};

    strncpy(Shader->PixelFileName, PixelShaderFileName, MAX_SHADER_FILENAME);
    strncpy(Shader->VertexFileName, VertexShaderFileName, MAX_SHADER_FILENAME);

    if(ShaderGroup->NumberOfShaders+1>= MAX_SHADER_NUMBER)
    {
	LogError("Not enough shaders allocated, failed to load shader program for %s, %s", VertexShaderFileName, PixelShaderFileName);
	return &NullShader;
    }
    MemoryMappedFile VertexShaderFile=MemoryMapFile(VertexShaderFileName);
    if(!VertexShaderFile.MMapBuffer)
    {
	LogError("Failed to load vertex shader file %s",VertexShaderFileName);
	return &NullShader;
    }

    MemoryMappedFile PixelShaderFile=MemoryMapFile(PixelShaderFileName);
    if(!PixelShaderFile.MMapBuffer)
    {
	UnMapFile(PixelShaderFile);
	LogError("Failed to load pixel shader file %s",PixelShaderFileName);
	return &NullShader;
    }

    Shader->ProgramID = glCreateProgram();
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("OpenGL error in CreateProgram for %s,%s : %s", VertexShaderFileName,PixelShaderFileName,gluErrorString(ErrorValue));
	Shader->ProgramID=0;
	UnMapFile(VertexShaderFile);
	UnMapFile(PixelShaderFile);
	return &NullShader;
    }
    Shader->VertexFileModifiedTime = VertexShaderFile.ModifiedTime;
    Shader->PixelFileModifiedTime = PixelShaderFile.ModifiedTime;

    Shader->VertexID = LoadShader(GL_VERTEX_SHADER,VertexShaderFile,VertexShaderFileName);
    glAttachShader(Shader->ProgramID,Shader->VertexID);

    Shader->PixelID = LoadShader(GL_FRAGMENT_SHADER,PixelShaderFile,PixelShaderFileName);
    glAttachShader(Shader->ProgramID,Shader->PixelID);

    UnMapFile(VertexShaderFile);
    UnMapFile(PixelShaderFile);

    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("OpenGL error in shader loading for %s,%s : %s", VertexShaderFileName,PixelShaderFileName,gluErrorString(ErrorValue));
	Shader->ProgramID=Shader->VertexID = Shader->PixelID = 0;
	return &NullShader;
    }

    if(!Shader->PixelID || !Shader->VertexID)
    {
	UnloadShaderProgram(Shader);
	Shader->ProgramID=Shader->VertexID = Shader->PixelID = 0;
	return &NullShader;
    }
    glLinkProgram(Shader->ProgramID);
    glUseProgram(Shader->ProgramID);

    return Shader;
}

inline GLint GetUniformLocation(ShaderProgram * Program, const char * UniformName)
{
    GLint Location=glGetUniformLocation(Program->ProgramID,UniformName);
    GLenum ErrorValue = glGetError();
    if(ErrorValue != GL_NO_ERROR)
    {
	LogError("Could not get uniform location of %s: %s",UniformName,gluErrorString(ErrorValue));
	return -1;
    }
    return Location;
}


union Color
{
    struct
    {
	float Red,Green,Blue;
    };
     struct
    {
	float R,G,B;
    };
    float Contents[3];
};


internal void DrawTexture2D(GLuint Texture, float X, float Y, float Width, float Height, Color Color, float Alpha, Texture2DShaderDetails * ShaderDetails, float U, float V, float TextureWidth, float TextureHeight)
{
    if(ShaderDetails->Program->ProgramID)
    {
	glUseProgram(ShaderDetails->Program->ProgramID);
	glUniform3fv(ShaderDetails->ColorLocation,1,Color.Contents);
	glUniform2f(ShaderDetails->PositionLocation,X,Y);
	glUniform1f(ShaderDetails->AlphaLocation,Alpha);
	glUniform2f(ShaderDetails->TextureOffsetLocation, U,V);
	glUniform2f(ShaderDetails->TextureSizeLocation, TextureWidth,TextureHeight);
	glUniform2f(ShaderDetails->SizeLocation,Width, Height);

	glBindVertexArray(ShaderDetails->VertexBuffer);
	glBindTexture(GL_TEXTURE_2D,Texture);
	glDrawArrays(GL_TRIANGLES,0,6);
    }
    else
    {
	static u8 Once = 1;
	if(Once)
	{
	    Once=0;
	    LogWarning("2D Texture shader not loaded, ignoring font rendering calls");
	}
    }
}

internal void DrawDebugRect(DebugRectShaderDetails * Details,float X, float Y, float Width, float Height, Color BorderColor , float BorderWidth , float BorderAlpha = 1.0f, Color Color ={{1,1,1}}, float InnerAlpha = 0.0f)
{
     if(Details->Program->ProgramID)
    {
	glUseProgram(Details->Program->ProgramID);
	glUniform4f(Details->ColorLocation,Color.R, Color.G, Color.B, InnerAlpha);
	glUniform2f(Details->PositionLocation,X,Y);
	glUniform1f(Details->BorderWidthLocation, BorderWidth);
	glUniform4f(Details->BorderColorLocation, BorderColor.R, BorderColor.G, BorderColor.B, BorderAlpha);
	glUniform2f(Details->SizeLocation,Width, Height);

	glBindVertexArray(Details->VertexBuffer);
	glDrawArrays(GL_TRIANGLES,0,6);
    }
    else
    {
	static u8 Once = 1;
	if(Once)
	{
	    Once=0;
	    LogWarning("Debug Rect shader not loaded, ignoring debug rect drawing calls");
	}
    }
}
