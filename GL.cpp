
const double PI = 3.14159265358979323846;

struct Matrix
{
    float Contents[16];
};


void SetProjectionMatrix(float VerticalFieldOfView,float AspectRatio,float NearPlane,float FarPlane,Matrix Projection)
{
    const float yscale=1.0/tan((VerticalFieldOfView/2)*PI/180.0),//cot(degtorad(vfow/2));
	xscale= yscale/AspectRatio,
	frustumlength=FarPlane-NearPlane;
    memset(Projection.Contents,0,16*sizeof(GLfloat));
    Projection.Contents[0*4+0]=xscale;
    Projection.Contents[1*4+1]=yscale;
    Projection.Contents[2*4+2]=-(FarPlane+NearPlane)/frustumlength;
    Projection.Contents[2*4+3]=-(2*NearPlane*FarPlane)/frustumlength;
    Projection.Contents[3*4+2]=-1;
}

void SetRotationMatrix(float x, float y, float z, float rad, Matrix Rotation)
{
    float vector[]={x,y,z};
    float vleninv=1/sqrt(x*x+y*y+z*z);
    for(int i=0;i<3;i++)
	vector[i]/=vleninv;
    float cosine=cos(rad);
    float sine=sin(rad);

    
    Rotation.Contents[0]=cosine+vector[0]*vector[0]*(1-cosine);
    Rotation.Contents[1]=vector[0]*vector[1]*(1-cosine)-vector[2]*sine;
    Rotation.Contents[2]=vector[0]*vector[2]*(1-cosine)+vector[1]*sine;

    Rotation.Contents[4]=vector[1]*vector[0]*(1-cosine)+vector[2]*sine;
    Rotation.Contents[5]=cosine+vector[1]*vector[1]*(1-cosine);
    Rotation.Contents[6]=vector[1]*vector[2]*(1-cosine)-vector[0]*sine;
	
    Rotation.Contents[8]=vector[2]*vector[0]*(1-cosine)-vector[1]*sine;
    Rotation.Contents[9]=vector[2]*vector[1]*(1-cosine)+vector[0]*sine;
    Rotation.Contents[10]=cosine+vector[2]*vector[2]*(1-cosine);
}

void SetTranslationMatrix(float x, float y, float z, Matrix Translation)
{
    float pos[]={x,y,z};
    for(int i=0;i<3;i++)
	Translation.Contents[i*4+3] = pos[i];   
}

inline Matrix operator*(Matrix A, Matrix B)
{
    Matrix result;
    for(int i=0;i<4;i++)
    {
	for(int j=0;j<4;j++)
	{
	    result.Contents[i*4+j]=0;
	    for(int k=0;k<4;k++)
		result.Contents[i*4+j]+=A.Contents[i*4+k]*B.Contents[k*4+j];
	}
    }
    return result;
}

bool UploadMatrix(Matrix Upload, GLuint Location)
{
    glUniformMatrix4fv(Location,1,GL_TRUE,Upload.Contents);
#ifdef __CSGL_DEBUG__
    GLenum Error = glGetError();
    if(Error!=GL_NO_ERROR)
    {
	LogError("Error Uploading Matrix: %s",gluErrorString(Error));
    }
#endif
    return true;
}

struct ShaderProgram
{
    GLuint ProgramID, VertexID, PixelID;
};


inline GLuint LoadShader(GLenum Type,MemoryMappedFile ShaderFile, const
			 char * FileName)
{
    GLuint ShaderID = glCreateShader(Type);
    GLint length=ShaderFile.FileSize;
    glShaderSource(ShaderID,1,(const char **)&ShaderFile.MMapBuffer,&length);
    glCompileShader(ShaderID);
    GLint ShaderSuccess;
    glGetShaderiv(ShaderID,GL_COMPILE_STATUS,&ShaderSuccess);
    if(ShaderSuccess != GL_TRUE)
    {
	int LogLen;
	glGetShaderiv(ShaderID,GL_INFO_LOG_LENGTH,&LogLen);
	GLchar ShaderLog[LogLen];
	glGetShaderInfoLog(ShaderID,LogLen,NULL,ShaderLog);
	LogError("Compile error while compiling shader %s:",FileName);
	LogDebug("%s",ShaderLog);
	return 0;
    }
    return ShaderID;
}

void UnloadShaderProgram(ShaderProgram Program)
{
    if(Program.ProgramID)
    {
	if(Program.VertexID)
	{
	    glDetachShader(Program.ProgramID,Program.VertexID);
	    glDeleteShader(Program.VertexID);
	}
	if(Program.PixelID)
	{
	    glDetachShader(Program.ProgramID,Program.PixelID);
	    glDeleteShader(Program.PixelID);
	}
	glDeleteProgram(Program.ProgramID);
    }
}

ShaderProgram LoadShaderProgram( const char * VertexShaderFileName, const char * PixelShaderFileName)
{
    MemoryMappedFile VertexShaderFile=MemoryMapFile(VertexShaderFileName);
    if(!VertexShaderFile.MMapBuffer)
    {
	LogError("Failed to load vertex shader file %s",VertexShaderFileName);
	return {0};
    }

    MemoryMappedFile PixelShaderFile=MemoryMapFile(PixelShaderFileName);
    if(!PixelShaderFile.MMapBuffer)
    {
	UnMapFile(PixelShaderFile);
	LogError("Failed to load pixel shader file %s",PixelShaderFileName);
	return {0};
    }
    
    ShaderProgram Program;
    Program.ProgramID = glCreateProgram();
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("OpenGL error in CreateProgram for %s,%s : %s", VertexShaderFileName,PixelShaderFileName,gluErrorString(ErrorValue));
    }
    Program.VertexID = LoadShader(GL_VERTEX_SHADER,VertexShaderFile,VertexShaderFileName);
    glAttachShader(Program.ProgramID,Program.VertexID);
    
    Program.PixelID = LoadShader(GL_FRAGMENT_SHADER,PixelShaderFile,PixelShaderFileName);
    glAttachShader(Program.ProgramID,Program.PixelID);

    UnMapFile(VertexShaderFile);
    UnMapFile(PixelShaderFile);
    
    if(!Program.PixelID || !Program.VertexID)
    {
	UnloadShaderProgram(Program);
	return {0};
    }
    glLinkProgram(Program.ProgramID);
    glUseProgram(Program.ProgramID);
    return Program;
}

inline GLuint GetUniformLocation(ShaderProgram Program, const char * UniformName)
{
    GLuint Location=glGetUniformLocation(Program.ProgramID,UniformName);
    GLenum ErrorValue = glGetError();
    if(ErrorValue != GL_NO_ERROR)
    {
	LogError("Could not get uniform location of %s: %s",UniformName,gluErrorString(ErrorValue));
	return -1;
    }
    return Location;
}
