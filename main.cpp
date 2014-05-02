#include <GL/glew.h>
#include <iostream>
#include <SDL2/SDL.h>

#include "lib/TriangleMesh.hpp"
#include "lib/Object.hpp"


#include "file_format/HPIFile.hpp"


bool quit = false;
Matrix ProjectionMatrix;
Object * TempObj[9];
ShaderProgram * DefaultShaders;


const int ScreenWidth=2560;
const int ScreenHeight=1440;

TriangleMesh * CreateCubeMesh();
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

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i=0;i<9;i++)
    {
	TempObj[i]->RotateY(30.0f/60.0f);
//	TempObj[i]->RotateX(60.0f/60.0f*(60.0f/framerate));
	TempObj[i]->Render();
    }
    
}


void setup()
{
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    ProjectionMatrix.SetProjectionMatrix(60,float(ScreenWidth)/ScreenHeight,1.0f,100.0f);
    
    
    DefaultShaders=new ShaderProgram("default");

    ProjectionMatrix.Upload(DefaultShaders->GetUniformLocation("ProjectionMatrix"));
    
    float CubeColor[]={1.0f,0.0f,0.0f};
    float CubePos[]={0.0f,0.0f,-12.0f};
    
    for(int i=0;i<9;i++)
    {
	CubePos[0]=i*2.0f-9.0f;
	TempObj[i]=new Object(CreateCubeMesh(),CubeColor,CubePos,Object::WhiteOutLine,0.5f);
	TempObj[i]->SetShader(DefaultShaders);

	// TempObj[i]->RotateZ(5.0f*i);
	TempObj[i]->RotateX(20);
	// TempObj[i]->RotateY(5.0f*i);
	TempObj[i]->Scale(i/10.0f+0.5f,0.5f,0.5f);
    }
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // ExitOnGLError("ERROR: Could not set OpenGL depth testing options");
 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    

}



int main(int argc, char **argv)
{
    HPI *hpi=new HPI("data/totala1.hpi");
    hpi->Print();
    delete hpi;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
    if (win == nullptr)
    {
	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	return 1;
    }

    SDL_GLContext gContext = SDL_GL_CreateContext( win );

    SDL_GL_MakeCurrent (win,gContext);
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
	std::cout<< "Warning: Unable to set VSync! SDL Error: "<<SDL_GetError()<<std::endl;
    }

    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
	std::cout<< "Error initializing GLEW! "<< glewGetErrorString( glewError )<<std::endl;
	return 1;
    }

    setup();


    
    

    //Event handler
    SDL_Event e;
		
    //Enable text input
    SDL_StartTextInput();

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
	    //Handle keypress with current mouse position
	    else if( e.type == SDL_TEXTINPUT )
	    {
		int x = 0, y = 0;
		SDL_GetMouseState( &x, &y );
		handleKeys( e.text.text[ 0 ], x, y );
	    }
	}

	//Render quad
	render();
			
	//Update screen
	SDL_GL_SwapWindow( win );
    }
    SDL_StopTextInput();
    
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
   
