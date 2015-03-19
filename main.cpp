#include <GL/glew.h>
#include <iostream>
#include <SDL2/SDL.h>



#include "TriangleMesh.hpp"
#include "Object.hpp"


#include "file_format/HPIFile.hpp"
#include "file_format/3DOFile.hpp"

bool quit = false;
Matrix ProjectionMatrix;
Object * TempObj[10];
ShaderProgram * DefaultShaders;
ShaderProgram * Shaders3DO;
HPI *hpi;

const int ScreenWidth=1280;
const int ScreenHeight=1024;

TriangleMesh * CreateCubeMesh();
TriangleMesh * CreateGeodesicSphere(int depth);

HPIDirectory * Objects3dDirectory=NULL;
int ObjFileIndex=0;

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message too
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg)
{
    os << msg << " error: " << SDL_GetError() << std::endl;
}

void handleKeys( unsigned char key, int x, int y )
{
	//Toggle quad
	if( key == 'q' )
	{
	    quit=true;
	}
}

Object * TempSphere;
GLuint ModelViewLocation;
Unit3DObject * ArmSolarObject;
Matrix ArmSolarMat;
GLuint RenderColorLocation;
extern uint8_t TAPalette[256*3];
GLfloat TAPaletteF[256*3];

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
const unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];
stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;
void my_stbtt_initfont(void)
{
   fread(ttf_buffer, 1, 1<<20, fopen("data/times.ttf", "rb"));
   stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, (stbtt_bakedchar*)cdata); // no guarantee this fits!
   // can free ttf_buffer at this point
   glGenTextures(1, &ftex);
   glBindTexture(GL_TEXTURE_2D, ftex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
   // can free temp_bitmap at this point
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void my_stbtt_print(float x, float y, char *text)
{
   // assume orthographic projection with units = screen pixels, origin at top left
   glBindTexture(GL_TEXTURE_2D, ftex);
   glBegin(GL_QUADS);
   while (*text) {
      if (*text >= 32 && *text < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
         glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y0);
         glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y0);
         glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y1);
         glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y1);
      }
      ++text;
   }
   glEnd();
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    

    Shaders3DO->Use();
    //ArmSolarMat.Rotate(0,1,0,0.01f);
    
    ArmSolarObject->Render(ArmSolarMat,ModelViewLocation);
    ArmSolarMat.Upload(ModelViewLocation);
    ProjectionMatrix.Upload(Shaders3DO->GetUniformLocation("ProjectionMatrix"));
    

    //DefaultShaders->Use();
    //  for(int i=0;i<10;i++)
    {
//	TempObj[i]->RotateY(30.0f/60.0f);
//	TempObj[i]->RotateX(60.0f/60.0f*(60.0f/framerate));
//	TempObj[i]->Render();
    }
    // TempObj[9]->Render();
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
	std::cout<<"failed to render : "<<gluErrorString(ErrorValue)<<std::endl;
    
    
    
    //TempSphere->RotateX(30.0f/60.0f);
    //TempSphere->Render();
    my_stbtt_print(0,0,"this is a TEST");
    
}

void SetViewport()
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float halfWidth = float(viewport[2]) / 2.0f;
    float halfHeight = float(viewport[3]) / 2.0f;

    float vpm[]={halfWidth,0,0,0,
			    0,halfHeight,0,0,
			    0,0,1,0,
	       halfWidth,halfHeight,0,1};
    Matrix ViewPortMatrix(vpm);
    ViewPortMatrix.Upload(DefaultShaders->GetUniformLocation("ViewPortMatrix"));
}

void LoadCurrent3doFile()
{

    HPIFile * FileToLoad=Objects3dDirectory->Files[ObjFileIndex];
    unsigned char * temp=new unsigned char[FileToLoad->GetData(nullptr)];
    FileToLoad->GetData(temp);
    ArmSolarObject=new Unit3DObject(temp);
}

void setup()
{
    my_stbtt_initfont();
    hpi=new HPI("data/totala1.hpi");
    LoadTextureList();
    
    Objects3dDirectory=hpi->GetDirectory("/objects3d");
        
    LoadCurrent3doFile();
    ArmSolarMat.Move(0,0,-8);

    
    glClearColor( 0.f, 0.f, 0.f, 0.f );
    ProjectionMatrix.SetProjectionMatrix(60,float(ScreenWidth)/ScreenHeight,1.0f,100.0f);
    

    Shaders3DO = new ShaderProgram("unit3do");
    ProjectionMatrix.Upload(Shaders3DO->GetUniformLocation("ProjectionMatrix"));
    ModelViewLocation=Shaders3DO->GetUniformLocation("ModelViewMatrix");
    glUniform1i(Shaders3DO->GetUniformLocation("UnitTexture"),0);
    RenderColorLocation=Shaders3DO->GetUniformLocation("RenderColor");
    
    for(int i=0;i<256*3;i++)
    {
	TAPaletteF[i]=TAPalette[i]/255.0f;
    }
   
    
    DefaultShaders=new ShaderProgram("default");
    SetViewport();

    ProjectionMatrix.Upload(DefaultShaders->GetUniformLocation("ProjectionMatrix"));
    
    float CubeColor[]={1.0f,0.0f,0.0f};
    float CubePos[]={0.0f,0.0f,-12.0f};
    
    for(int i=0;i<9;i++)
    {
	CubePos[0]=i*2.0f-9.0f;
	CubeColor[2]=i/9.0f;
	TempObj[i]=new Object(CreateCubeMesh(),CubeColor,CubePos,Object::WhiteOutLine,1.0f+i/2.5f);
	TempObj[i]->SetShader(DefaultShaders);

	// TempObj[i]->RotateZ(5.0f*i);
	TempObj[i]->RotateX(20);
	// TempObj[i]->RotateY(5.0f*i);
	TempObj[i]->Scale(i/10.0f+0.5f,0.5f,0.5f);
    }
    CubeColor[2]=0;
    CubePos[0]=0;
    CubePos[1]=0;
    CubePos[2]=-18;
    TempObj[9]=new Object(CreateCubeMesh(),CubeColor,CubePos,Object::WhiteOutLine,2.0f);
    TempObj[9]->SetShader(DefaultShaders);

    TempObj[9]->SetPos(0,0,-55.0f);
    TempObj[9]->ResetScale();
    TempObj[9]->Scale(10,10,10);

    
    CubePos[0]=-0.5f;
    CubePos[1]=0;
    CubePos[2]=-8.0f;
    float SphereColor[]={0.0f,1.0f,0.0f};
    TempSphere=new Object(CreateGeodesicSphere(2),SphereColor,CubePos,Object::WhiteOutLine,3.0f);
    TempSphere->SetShader(DefaultShaders);
    TempSphere->Scale(1.0,1.0,0.5);
    TempSphere->SetPos(0,3,-8);
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // ExitOnGLError("ERROR: Could not set OpenGL depth testing options");
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    

}

void HandleKeyDown(SDL_Keysym key)
{
    switch(key.sym)
    {
    case SDLK_l:
	ObjFileIndex++;
	if(ObjFileIndex >= Objects3dDirectory->NumFiles)
	    ObjFileIndex = Objects3dDirectory->NumFiles-1;
	LoadCurrent3doFile();
	break;
    case SDLK_o:
	ObjFileIndex--;
	if(ObjFileIndex < 0)
	    ObjFileIndex=0;
	LoadCurrent3doFile();
	break;
    case SDLK_ESCAPE:
	quit=true;
	break;
    case SDLK_UP:
	ArmSolarMat.Rotate(1,0,0,-0.01f);
	break;
    case SDLK_DOWN:
	ArmSolarMat.Rotate(1,0,0,0.01f);
	break;
    case SDLK_LEFT:
	ArmSolarMat.Rotate(0,1,0,-0.01f);
	break;
    case SDLK_RIGHT:
	ArmSolarMat.Rotate(0,1,0,0.01f);
	break;
    case SDLK_KP_PLUS:
    case SDLK_PLUS:
	ArmSolarMat.Move(0,0,0.1f);
	break;
    case SDLK_KP_MINUS:
    case SDLK_MINUS:
	ArmSolarMat.Move(0,0,-0.1f);
	break;
    }
}



int main(int argc, char **argv)
{
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    if (win == nullptr)
    {
	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	return 1;
    }


    GLint GLMajorVer, GLMinorVer;
    
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
	std::cout<<"failed before create context : "<<gluErrorString(ErrorValue)<<std::endl;
    

    SDL_GLContext gContext = SDL_GL_CreateContext( win );
    SDL_GL_MakeCurrent (win,gContext);
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
	std::cout<< "Warning: Unable to set VSync! SDL Error: "<<SDL_GetError()<<std::endl;
    }

    glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVer);
    glGetIntegerv(GL_MINOR_VERSION, &GLMinorVer);
    std::cout<<"OpenGL Version "<<GLMajorVer<<"."<<GLMinorVer<<std::endl;


    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
	std::cout<<"failed before glewInit : "<<gluErrorString(ErrorValue)<<std::endl;
  

    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
	std::cout<< "Error initializing GLEW! "<< glewGetErrorString( glewError )<<std::endl;
	return 1;
    }

    
    //as we need to use glewExperimental - known issue (it segfaults otherwise!) - we encounter
    //another known issue, which is that while glewInit suceeds, it leaves opengl in an error state
    ErrorValue = glGetError();
    
    
    setup();


    
    

    //Event handler
    SDL_Event e;
		
    //Enable text input
    //SDL_StartTextInput();

    //While application is running
    while( !quit )
    {
	//Handle events on queue
	while( SDL_PollEvent( &e ) != 0 )
	{
	    //User requests quit
	    if( e.type == SDL_QUIT )
	    {
		quit = true;
	    }
	    else if( e.type == SDL_KEYDOWN)
	    {
		HandleKeyDown(e.key.keysym);
	    }
	}

	//Render quad
	render();
			
	//Update screen
	SDL_GL_SwapWindow( win );
    }
    //SDL_StopTextInput();
    delete hpi;
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
   
