#include <GL/glew.h>
#include "Object.hpp"

const float Object::BlackOutLine[]={0.0f,0.0f,0.0f};
const float Object::WhiteOutLine[]={1.0f,1.0f,1.0f};


void Object::Render(bool SetMesh)
{
    Matrix M=PositionMatrix*RotationMatrix*ScaleMatrix;
    M.Upload(ModelMatrixLocation);
    M.Upload3x3Rotation(NormalMatrixLocation);
	
    glUniform3fv(MeshColorLocation,1,Color);
    glUniform3fv(OutlineColorLocation,1,OutlineColor);
    glUniform1f(LineWidthLocation,LineWidth);
    if(SetMesh)
	Mesh->SetMesh();
    Mesh->Render();
}
Object::Object(TriangleMesh * mesh,float color[],float position[],const float outlinecolor[],const float outlinewidth):Shader(0),Mesh(mesh),LineWidth(outlinewidth)
{
    Mesh->AddRef();
    Color[0]=color[0];
    Color[1]=color[1];
    Color[2]=color[2];

    Position[0]=position[0];
    Position[1]=position[1];
    Position[2]=position[2];

    OutlineColor[0]=outlinecolor[0];
    OutlineColor[1]=outlinecolor[1];
    OutlineColor[2]=outlinecolor[2];

    PositionMatrix.SetPosition(Position);
}
Object::~Object()
{
    if(!Mesh->RemoveRef())
	delete Mesh;
}
void Object::RotateX(float Degrees)
{
    RotationMatrix.Rotate(1,0,0,Degrees*PI/180);
}
void Object::RotateY(float Degrees)
{
    RotationMatrix.Rotate(0,1,0,Degrees*PI/180);
}
void Object::RotateZ(float Degrees)
{
    RotationMatrix.Rotate(0,1,0,Degrees*PI/180);
}
void Object::SetPos(float x,float y,float z)
{
    Position[0]=x;
    Position[1]=y;
    Position[2]=z;
    PositionMatrix.SetPosition(Position);
}
void Object::Move(float dx,float dy,float dz)
{
    PositionMatrix.Move(dx,dy,dz);
}
void Object::ResetRotation()
{
    RotationMatrix=Matrix();
}
void Object::Scale(float dx,float dy, float dz)
{
    ScaleMatrix.Scale(dx,dy,dz);
}
void Object::ResetScale()
{
    ScaleMatrix=Matrix();
}
void Object::SetShader(ShaderProgram * sp)
{
    if(Shader)
    {

    }
    Shader=sp;
    MeshColorLocation=Shader->GetUniformLocation("MeshColor");
    OutlineColorLocation=Shader->GetUniformLocation("LineColor");
    LineWidthLocation=Shader->GetUniformLocation("LineWidth");
    ModelMatrixLocation=Shader->GetUniformLocation("ModelMatrix");
    NormalMatrixLocation=Shader->GetUniformLocation("NormalMatrix");
}

