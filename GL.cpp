




struct Matrix
{
    float Contents[16];
    void SetIdentity()
    {
	memset(Contents,0,16*sizeof(GLfloat));
	for(int i=0;i<4;i++)
	    Contents[i*4+i]=1;
    }
    
    void SetProjection(float VerticalFieldOfView,float AspectRatio,float NearPlane,float FarPlane)
    {
	SetIdentity();
	const float yscale=1.0f/(float)tan((VerticalFieldOfView/2)*PI/180.0f),//cot(degtorad(vfow/2));
	    xscale= yscale/AspectRatio,
	    frustumlength=FarPlane-NearPlane;
	memset(Contents,0,16*sizeof(GLfloat));
	Contents[0*4+0]=xscale;
	Contents[1*4+1]=yscale;
	Contents[2*4+2]=-(FarPlane+NearPlane)/frustumlength;
	Contents[2*4+3]=-(2*NearPlane*FarPlane)/frustumlength;
	Contents[3*4+2]=-1;
    }
    
    void SetTranslation(float x, float y, float z)
    {
	SetIdentity();
	float pos[]={x,y,z};
	for(int i=0;i<3;i++)
	    Contents[i*4+3] = pos[i];   
    }
    
    void SetRotation(float x, float y, float z, float rad)
    {
	SetIdentity();
	float vector[]={x,y,z};
	float vleninv=1/sqrt(x*x+y*y+z*z);
	for(int i=0;i<3;i++)
	    vector[i]/=vleninv;
	float cosine=cos(rad);
	float sine=sin(rad);

    
	Contents[0]=cosine+vector[0]*vector[0]*(1-cosine);
	Contents[1]=vector[0]*vector[1]*(1-cosine)-vector[2]*sine;
	Contents[2]=vector[0]*vector[2]*(1-cosine)+vector[1]*sine;

	Contents[4]=vector[1]*vector[0]*(1-cosine)+vector[2]*sine;
	Contents[5]=cosine+vector[1]*vector[1]*(1-cosine);
	Contents[6]=vector[1]*vector[2]*(1-cosine)-vector[0]*sine;
	
	Contents[8]=vector[2]*vector[0]*(1-cosine)-vector[1]*sine;
	Contents[9]=vector[2]*vector[1]*(1-cosine)+vector[0]*sine;
	Contents[10]=cosine+vector[2]*vector[2]*(1-cosine);
    }

    bool Upload(GLuint Location)
    {
	glUniformMatrix4fv(Location,1,GL_TRUE,Contents);
#ifdef __CSGL_DEBUG__
	GLenum Error = glGetError();
	if(Error!=GL_NO_ERROR)
	{
	    LogError("Error Uploading Matrix: %s",gluErrorString(Error));
	}
#endif
	return true;
    }


    void Scale(float x,float y,float z)
    {
	float vector[]={x,y,z};
	Matrix temp=Matrix();
	for(int i=0;i<3;i++)
	    temp.Contents[i*4+i]=vector[i];
	*this*=temp;
    }

    void SetScale(float x, float y, float z)
    {
	SetIdentity();
	float vector[]={x,y,z};
	for(int i=0;i<3;i++)
	    Contents[i*4+i]=vector[i];
    }

    void Rotate(float x,float y,float z, float rad)
    {
	float vector[]={x,y,z};
	float vleninv=1/sqrt(x*x+y*y+z*z);
	for(int i=0;i<3;i++)
	    vector[i]/=vleninv;
	float cosine=cos(rad);
	float sine=sin(rad);

	Matrix temp;
	temp.Contents[0]=cosine+vector[0]*vector[0]*(1-cosine);
	temp.Contents[1]=vector[0]*vector[1]*(1-cosine)-vector[2]*sine;
	temp.Contents[2]=vector[0]*vector[2]*(1-cosine)+vector[1]*sine;

	temp.Contents[4]=vector[1]*vector[0]*(1-cosine)+vector[2]*sine;
	temp.Contents[5]=cosine+vector[1]*vector[1]*(1-cosine);
	temp.Contents[6]=vector[1]*vector[2]*(1-cosine)-vector[0]*sine;
	
	temp.Contents[8]=vector[2]*vector[0]*(1-cosine)-vector[1]*sine;
	temp.Contents[9]=vector[2]*vector[1]*(1-cosine)+vector[0]*sine;
	temp.Contents[10]=cosine+vector[2]*vector[2]*(1-cosine);

	//*this = temp**this;
	*this *= temp;
    }
    
    void Move(float dx,float dy, float dz)
    {
	float pos[]={dx,dy,dz};
	for(int i=0;i<3;i++)
	    Contents[i*4+3]+=pos[i];
    }

    Matrix & operator*= (const Matrix & other)
    {
	float temp[16];
	for(int i=0;i<4;i++)
	    for(int j=0;j<4;j++)
	    {
		temp[i*4+j]=0;
		for(int k=0;k<4;k++)
		    temp[i*4+j]+=Contents[i*4+k]*other.Contents[k*4+j];
	    }
	for(int i=0;i<16;i++)
	    Contents[i]=temp[i];
	return *this;
    }
    
    Matrix operator* (const Matrix & other)
    {
	Matrix ret;
	for(int i=0;i<4;i++)
	    for(int j=0;j<4;j++)
	    {
		ret.Contents[i*4+j]=0;
		for(int k=0;k<4;k++)
		    ret.Contents[i*4+j]+=Contents[i*4+k]*other.Contents[k*4+j];
	    }
	return Matrix(ret);
    }
    
    Matrix()
    {
	SetIdentity();
    }

    Matrix(const Matrix & other)
    {
	for(int i=0;i<16;i++)
	    Contents[i]=other.Contents[i];
    }
    
    Matrix(const GLfloat array[])
    {
	for(int i=0;i<16;i++)
	    Contents[i]=array[i];
    }
    
    Matrix & operator= (const Matrix & other)
    {
	if(this != &other)
	{
	    for(int i=0;i<16;i++)
		Contents[i]=other.Contents[i];
	}
	return *this;
    }
};






struct ShaderProgram
{
    GLuint ProgramID, VertexID, PixelID;
    uint64_t VertexFileModifiedTime, PixelFileModifiedTime;
    char PixelFileName[MAX_SHADER_FILENAME];
    char VertexFileName[MAX_SHADER_FILENAME];
};

static ShaderProgram NullShader
= {0,0,0,0,0,0,0};


inline GLuint LoadShader(GLenum Type,MemoryMappedFile ShaderFile, const
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
	int LogLen;
	glGetShaderiv(ShaderID,GL_INFO_LOG_LENGTH,&LogLen);
	//GLchar * ShaderLog = (GLchar *)alloca(LogLen);
	STACK_ARRAY(ShaderLog, LogLen, GLchar);
	glGetShaderInfoLog(ShaderID,LogLen,NULL,ShaderLog);
	LogError("Compile error while compiling shader %s:",FileName);
	LogDebug("%s",ShaderLog);
	return 0;
    }
    return ShaderID;
}

void UnloadShaderProgram(ShaderProgram * Program)
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

void UnloadAllShaders(GameState * CurrentGameState)
{
    for(int i=0;i<CurrentGameState->NumberOfShaders;i++)
    {
	UnloadShaderProgram(&CurrentGameState->Shaders[i]);
    }
    CurrentGameState->NumberOfShaders=0;
}

ShaderProgram * LoadShaderProgram( const char * VertexShaderFileName, const char * PixelShaderFileName, GameState * CurrentGameState)
{
    ShaderProgram * Shader = &CurrentGameState->Shaders[CurrentGameState->NumberOfShaders++];
    *Shader = {};
   
    strncpy(Shader->PixelFileName, PixelShaderFileName, MAX_SHADER_FILENAME);
    strncpy(Shader->VertexFileName, VertexShaderFileName, MAX_SHADER_FILENAME);
   
    if(CurrentGameState->NumberOfShaders+1>= MAX_SHADER_NUMBER)
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


struct Texture2DShaderDetails
{
    GLuint ColorLocation, AlphaLocation, PositionLocation, SizeLocation, TextureOffsetLocation, TextureSizeLocation;
    ShaderProgram Program;
};
