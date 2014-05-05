

#include "Shader.hpp"
#include <string.h>

Shader::Shader(FILE * ShaderFile,GLuint ProgramID,int Type,std::string name)
{
    fseek(ShaderFile, 0,SEEK_END);
    int length=ftell(ShaderFile);
    fseek(ShaderFile,0,SEEK_SET);
    char *Contents = new char[length+1];

    memset(Contents,0,length+1);

    int readbytes=fread(Contents,1,length,ShaderFile);
    if(readbytes!=length)
    {
	std::cout<<"Only read "<<readbytes<<"( of "<<length<<"bytes) of "<<name<<std::endl;
	delete [] Contents;
	return;
    }
    Contents[length]=0;

    ID=glCreateShader(Type);
    char ** temp=&Contents;
    glShaderSource(ID,1,(const char **)temp,&length);
    glCompileShader(ID);
    GLint ShaderSuccess;
    glGetShaderiv(ID,GL_COMPILE_STATUS,&ShaderSuccess);
    if(ShaderSuccess!=GL_TRUE)
    {
	int LogLen;
	glGetShaderiv(ID,GL_INFO_LOG_LENGTH,&LogLen);
	GLchar * ShaderLog=new GLchar[LogLen];
	glGetShaderInfoLog(ID,LogLen,NULL,ShaderLog);
	std::cout<<name<<" failed to compile "<<Contents<<std::endl<<ShaderLog<<std::endl;
	delete [] ShaderLog;
    }
    else
	std::cout<<"Successfully compiled "<<name<<std::endl;
    glAttachShader(ProgramID,ID);

    delete [] Contents;
}

void Shader::Detach(GLuint ProgramID)
{
    glDetachShader(ProgramID,ID);
}
Shader::~Shader()
{
    glDeleteShader(ID);
}

const std::string ShaderExtensions[]={".vs.glsl",".tc.glsl",".te.glsl",".gs.glsl",".fs.glsl"};
const int ShaderTypes[]={GL_VERTEX_SHADER,GL_TESS_CONTROL_SHADER,GL_TESS_EVALUATION_SHADER,GL_GEOMETRY_SHADER,GL_FRAGMENT_SHADER};

//const int NumShaderTypes=sizeof(ShaderTypes)/sizeof(ShaderTypes[0]);


GLuint ShaderProgram::GetUniformLocation(std::string name)
{
    GLuint ret= glGetUniformLocation(ID,name.c_str());
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
	std::cout<<"failed to get uniform location of "<<name<<" : "<<gluErrorString(ErrorValue)<<std::endl;
    return ret;
}

void ShaderProgram::LoadShaders(std::string ShaderNames[])
{
    for(int i=0;i<NumShaderTypes;i++)
    {
	FILE * ShaderFile=fopen(("shaders/"+ShaderNames[i]).c_str(),"rb");
	if(ShaderFile)
	{
	    Shaders[i]=new Shader(ShaderFile,ID,ShaderTypes[i],ShaderNames[i]);
	    fclose(ShaderFile);
	}
	else
	    Shaders[i]=nullptr;
    }
    glLinkProgram(ID);
    glUseProgram(ID);
}

ShaderProgram::ShaderProgram(std::string VSName,std::string FSName,std::string GSName, std::string TCSName,std::string TESName)
{
    ID=glCreateProgram();
    std::string ShaderNames[]={VSName,FSName,GSName,TCSName,TESName};
    LoadShaders(ShaderNames);
}

ShaderProgram::ShaderProgram(std::string name)
{
    ID=glCreateProgram();
    std::string ShaderNames[NumShaderTypes];
    for(int i=0;i<NumShaderTypes;i++)
    {
	ShaderNames[i]=name+ShaderExtensions[i];
    }
    LoadShaders(ShaderNames);
}
ShaderProgram::~ShaderProgram()
{
    glUseProgram(0);//for now, really should fix to only do if in use
    for(int i=0;i<NumShaderTypes;i++)
    {
	if(Shaders[i])
	{
	    Shaders[i]->Detach(ID);
	    delete Shaders[i];
	}
    }
    glDeleteProgram(ID);
}

