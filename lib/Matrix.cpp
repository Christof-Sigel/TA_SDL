#include <GL/glxew.h>
#include <cmath>
#include "Matrix.hpp"
#include <string.h>


void Matrix::Scale(float x,float y,float z)
{
    float vector[]={x,y,z};
    Matrix temp=Matrix();
    for(int i=0;i<3;i++)
	temp.Contents[i*4+i]=vector[i];
    *this*=temp;
}

void Matrix::SetProjectionMatrix(float VerticalFieldOfView,float AspectRatio,float NearPlane,float FarPlane)
{
    const float yscale=1.0/tan((VerticalFieldOfView/2)*PI/180.0),//cot(degtorad(vfow/2));
	xscale= yscale/AspectRatio,
	frustumlength=FarPlane-NearPlane;
    memset(Contents,0,16*sizeof(GLfloat));
    Contents[0*4+0]=xscale;
    Contents[1*4+1]=yscale;
    Contents[2*4+2]=-(FarPlane+NearPlane)/frustumlength;
    Contents[2*4+3]=-(2*NearPlane*FarPlane)/frustumlength;
    Contents[3*4+2]=-1;
}
	
void Matrix::Rotate(float x,float y,float z, float rad)
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

    *this*=temp;	
}
void Matrix::Move(float dx,float dy, float dz)
{
    float pos[]={dx,dy,dz};
    for(int i=0;i<3;i++)
	Contents[i*4+3]+=pos[i];
}
void Matrix::SetPosition(float Position[])
{
    for(int i=0;i<3;i++)
	Contents[i*4+3]=Position[i];
}
Matrix::Matrix()
{
    memset(Contents,0,16*sizeof(GLfloat));
    for(int i=0;i<4;i++)
	Contents[i*4+i]=1;
}
Matrix::Matrix(const Matrix & other)
{
    for(int i=0;i<16;i++)
	Contents[i]=other.Contents[i];
}
Matrix::Matrix(const GLfloat array[])
{
    for(int i=0;i<16;i++)
	Contents[i]=array[i];
}
Matrix & Matrix::operator= (const Matrix & other)
{
    if(this != &other)
    {
	for(int i=0;i<16;i++)
	    Contents[i]=other.Contents[i];
    }
    return *this;
}
bool Matrix::Upload(GLuint Location)
{
    glUniformMatrix4fv(Location,1,GL_TRUE,Contents);//Opengl uses column major order, c row major, so the matrix needs to be transposed
#ifdef __SIMSCHOOL_DEBUG_
    GLenum Error=glGetError();
    if(Error!=GL_NO_ERROR)
    {
	std::cout<<"Error Uploading Matrix : "<<gluErrorString(Error)<<std::endl;
	return false;
    }
#endif
    return true;
}
bool Matrix::Upload3x3Rotation(GLuint Location)
{
    float U3x3[9];
    for(int i=0;i<9;i++)
	U3x3[i]=Contents[(i/3)*4+i%3];
    glUniformMatrix3fv(Location,1,GL_TRUE,U3x3);
#ifdef __SIMSCHOOL_DEBUG_
    GLenum Error=glGetError();
    if(Error!=GL_NO_ERROR)
    {
	std::cout<<"Error Uploading Matrix : "<<gluErrorString(Error)<<std::endl;
	return false;
    }
#endif
    return true;
}
Matrix & Matrix::operator*= (const Matrix & other)
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
Matrix Matrix::operator* (const Matrix & other)
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

