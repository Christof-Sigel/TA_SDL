#ifndef __SIMSCHOOL_MATRIX_H
#define __SIMSCHOOL_MATRIX_H

const double PI = 3.14159265358979323846;

class Matrix
{
public:
    void Scale(float x,float y,float z);
    void SetProjectionMatrix(float VerticalFieldOfView,float AspectRatio,float NearPlane,float FarPlane);
    void Rotate(float x,float y,float z, float rad);
    void Move(float dx,float dy, float dz);
    void SetPosition(float Position[]);
    Matrix();
    Matrix(const Matrix & other);
    Matrix(const GLfloat array[]);
    Matrix & operator= (const Matrix & other);
    bool Upload(GLuint Location);
    bool Upload3x3Rotation(GLuint Location);
    Matrix & operator*= (const Matrix & other);
    Matrix operator* (const Matrix & other);
private:
    GLfloat Contents[16];
};

#endif
