#ifndef __CSGL_OBJECT_H
#define __CSGL_OBJECT_H

#include "TriangleMesh.hpp"
#include "Matrix.hpp"
class Object
{
public:
    const static float BlackOutLine[];
    const static float WhiteOutLine[];
    void Render(bool SetMesh=true);
    Object(TriangleMesh * mesh,float color[],float position[],const float outlinecolor[]=BlackOutLine,const float outlinewidth=1.0f);
    ~Object();
    void RotateX(float Degrees);
    void RotateY(float Degrees);
    void RotateZ(float Degrees);
    void SetPos(float x,float y,float z);
    void Move(float dx,float dy,float dz);
    void ResetRotation();
    void Scale(float dx,float dy, float dz);
    void ResetScale();
    void SetShader(ShaderProgram * sp);
private:
    ShaderProgram * Shader;
    TriangleMesh * Mesh;
    float Color[3],Position[3],OutlineColor[3],LineWidth;
    GLuint MeshColorLocation,OutlineColorLocation,LineWidthLocation,ModelMatrixLocation,NormalMatrixLocation;

    Matrix RotationMatrix,PositionMatrix,ScaleMatrix;
};


#endif
