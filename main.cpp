#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "lib/TriangleMesh.hpp"

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message too
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg)
{
    os << msg << " error: " << SDL_GetError() << std::endl;
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr)
    {
	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_Renderer * ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr)
    {
	std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_Surface *bmp = SDL_LoadBMP("data/hello.bmp");
    if (bmp == nullptr)
    {
	std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
    SDL_FreeSurface(bmp);
    if (tex == nullptr)
    {
	std::cout << "SDL_CreateTextureFromSurface Error: "
		  << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_Delay(2000);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();


    return 0;
}

void Normalize(GLfloat v[3])
{
    float size=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    for(int i=0;i<3;i++)
	v[i]/=size;
}


void SubDivideSphere(GLfloat Vertices[],int depth)
{
    if(depth==0)
	return;
    int newvertoffset=1;
    if(depth>=2)
	newvertoffset=pow(4,depth-2)*3;
    int vertoffset= newvertoffset*4;
    if(depth==1)
	vertoffset=3;

    vertoffset*=3;
    newvertoffset*=3;
    GLfloat * v1=&Vertices[vertoffset*0];
    GLfloat * v2=&Vertices[vertoffset*1];
    GLfloat * v3=&Vertices[vertoffset*2];
    GLfloat v12[]={(v1[0]+v2[0])/2.0f,(v1[1]+v2[1])/2.0f,(v1[2]+v2[2])/2.0f};
    GLfloat v23[]={(v2[0]+v3[0])/2.0f,(v2[1]+v3[1])/2.0f,(v2[2]+v3[2])/2.0f};
    GLfloat v31[]={(v1[0]+v3[0])/2.0f,(v1[1]+v3[1])/2.0f,(v1[2]+v3[2])/2.0f};

    Normalize(v12);
    Normalize(v23);
    Normalize(v31);
    memcpy(&Vertices[(vertoffset*0+newvertoffset*1)],v12,sizeof(GLfloat)*3);
    memcpy(&Vertices[(vertoffset*0+newvertoffset*2)],v31,sizeof(GLfloat)*3);
    
    memcpy(&Vertices[(vertoffset*1+newvertoffset*1)],v23,sizeof(GLfloat)*3);
    memcpy(&Vertices[(vertoffset*1+newvertoffset*2)],v12,sizeof(GLfloat)*3);

    memcpy(&Vertices[(vertoffset*2+newvertoffset*1)],v31,sizeof(GLfloat)*3);
    memcpy(&Vertices[(vertoffset*2+newvertoffset*2)],v23,sizeof(GLfloat)*3);

    memcpy(&Vertices[(vertoffset*3+newvertoffset*0)],v12,sizeof(GLfloat)*3);
    memcpy(&Vertices[(vertoffset*3+newvertoffset*1)],v23,sizeof(GLfloat)*3);
    memcpy(&Vertices[(vertoffset*3+newvertoffset*2)],v31,sizeof(GLfloat)*3);

    for(int i=0;i<4;i++)
    {
	SubDivideSphere(&Vertices[(vertoffset*i)],depth-1);
    }
    
}


TriangleMesh * CreateGeodesicSphere(int depth)
{

    const float X = 0.525731112119133606f;
    const float Z = 0.850650808352039932f;
    
    float vertices[] ={-X,0.0f,Z,  X,0.0f,Z,   -X,0.0f,-Z,  X,0.0f,-Z,
		       0.0f,Z,X,   0.0f,Z,-X,  0.0f,-Z,X,  0.0f,-Z,-X,
		       Z,X,0.0,   -Z,X,0.0f,  Z,-X,0.0f,  -Z,-X,0.0f};
    
    GLuint indices[] ={1,4,0,   4,9,0,  4,5,9,  8,5,4,  1,8,4,
		    1,10,8,  10,3,8,  8,3,5,  3,2,5,  3,7,2,
		    3,10,7,  10,6,7,  6,11,7,  6,0,11,  6,1,0,
		    10,1,6,  11,0,9,  2,11,9,  5,2,9,  11,2,7};

    GLuint NumVertices=sizeof(indices)/sizeof(GLuint);
    int Multiplier=pow(4,depth);
    GLfloat * Vertices=new GLfloat[NumVertices*3*Multiplier];
   
    for(GLuint i=0;i<NumVertices/3;i++)
    {
	for(int j=0;j<3;j++)
	{
	    Vertices[i*3*3*Multiplier+0+j*((Multiplier/4)*3)*3]=vertices[indices[i*3+j]*3+0];
	    Vertices[i*3*3*Multiplier+1+j*((Multiplier/4)*3)*3]=vertices[indices[i*3+j]*3+1];
	    Vertices[i*3*3*Multiplier+2+j*((Multiplier/4)*3)*3]=vertices[indices[i*3+j]*3+2];
	}

    }
    for(GLuint i=0;i<NumVertices/3;i++)
	SubDivideSphere(&Vertices[i*3*3*Multiplier],depth);    
    
    TriangleMesh * SphereMesh=new TriangleMesh(NumVertices*Multiplier,Vertices,Vertices);
    return SphereMesh;
}

TriangleMesh * CreateCubeMesh()
{
    static TriangleMesh * CubeMesh=0;
    if(!CubeMesh)
    {
	float vertices[]= {
	    -.5f, -.5f,  .5f, //0
	    -.5f,  .5f,  .5f,  //1
	    .5f,  .5f,  .5f, //2
	    .5f, -.5f,  .5f, //3
	    -.5f, -.5f, -.5f , //4
	    -.5f,  .5f, -.5f , //5
	    .5f,  .5f, -.5f ,  //6
	    .5f, -.5f, -.5f //7
	};
	    
	const float normals[]={
	    0,0,1,
	    0,-1,0,
	    -1,0,0,
	    1,0,0,
	    0,1,0,
	    0,0,-1
	};

	const GLuint indices[36] =
	    {
		0,2,1,  0,3,2,
		4,3,0,  4,7,3,
		4,1,5,  4,0,1,
		3,6,2,  3,7,6,
		1,6,5,  1,2,6,
		7,5,6,  7,4,5
	    };

    
	int NumVertices=sizeof(indices)/sizeof(GLuint);
	GLfloat * Vertices=new GLfloat[NumVertices*3];
	GLfloat * Normals=new GLfloat[NumVertices*3];

	for(int i=0;i<NumVertices;i++)
	{
	    Vertices[3*i+0]=vertices[indices[i]*3+0];
	    Vertices[3*i+1]=vertices[indices[i]*3+1];
	    Vertices[3*i+2]=vertices[indices[i]*3+2];
	
	    Normals[3*i+0]=normals[(i/6)*3+0];
	    Normals[3*i+1]=normals[(i/6)*3+1];
	    Normals[3*i+2]=normals[(i/6)*3+2];
	}

	CubeMesh = new TriangleMesh(NumVertices,Vertices,Normals);
    }
    return CubeMesh;
}
   
