#ifndef __SIMSCHOOL_SHADER
#define __SIMSCHOOL_SHADER

#include <GL/glew.h>
#include <string>
#include <stdio.h>


#include <iostream>

const int NumShaderTypes=5;

class Shader
{
public:
    Shader(FILE * ShaderFile,GLuint ProgramID,int Type,std::string name);
    void Detach(GLuint ProgramID);
    ~Shader();

private:
    GLuint ID;

};

enum ShaderType
{
    SHADER_GEOMETRY=0,
    SHADER_TESS_CONTROL,
    SHADER_TESS_EVAL,
    SHADER_VERTEX,
    SHADER_FRAGMENT
};
    

class ShaderProgram
{
public:
    GLuint GetUniformLocation(std::string name);
    ShaderProgram(std::string name);
    ShaderProgram(std::string VertexShaderName,std::string FragmentShaderName,std::string GeometryShaderName="",std::string TesselationControlShaderName="",std::string TesselationEvalShaderName="");
    ~ShaderProgram();
   private:
    void LoadShaders(std::string ShaderNames[]);
    GLuint ID;
    Shader *Shaders[NumShaderTypes];
};



#endif
