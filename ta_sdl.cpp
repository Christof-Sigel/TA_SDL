#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>

typedef int32_t bool32;


#include "platform_code.cpp"
#include "GL.cpp"
#include "file_formats.cpp"

void HandleKeyDown(SDL_Keysym key);
bool32 quit=0;
bool32 SetupSDLWindow();
void Render();
void Setup();
void Teardown();
SDL_Window * MainSDLWindow;

int ScreenWidth=1024,ScreenHeight=768;

int main(int argc, char * argv[])
{
    if(!SetupSDLWindow())
	return 1;

    Setup();
    
    SDL_Event e;
    while( !quit )
    {
	while( SDL_PollEvent( &e ) != 0 )
	{
	    if( e.type == SDL_QUIT )
	    {
		quit = true;
	    }
	    else if( e.type == SDL_KEYDOWN)
	    {
		HandleKeyDown(e.key.keysym);
	    }
	}
	Render();
	SDL_GL_SwapWindow( MainSDLWindow );
    }

    Teardown();
    SDL_DestroyWindow(MainSDLWindow);
    SDL_Quit();

    return 0;
}


void PrintHPIDirectory(HPIDirectoryEntry dir, int Tabs=0)
{
    char printf_format[Tabs+2+3+2+1+1];
    for(int i=0;i<Tabs;i++)
	printf_format[i]='\t';
    printf_format[Tabs]='%';
    printf_format[Tabs+1]='s';
    printf_format[Tabs+2]=' ';
    printf_format[Tabs+3]='-';
    printf_format[Tabs+4]=' ';
    printf_format[Tabs+5]='%';
    printf_format[Tabs+6]='d';
    printf_format[Tabs+7]='\n';
    printf_format[Tabs+8]=0;

    for(int EntryIndex=0;EntryIndex<dir.NumberOfEntries;EntryIndex++)
    {
	printf(printf_format,dir.Entries[EntryIndex].Name,dir.Entries[EntryIndex].IsDirectory?0:dir.Entries[EntryIndex].File.FileSize);
	if(dir.Entries[EntryIndex].IsDirectory)
	{
	    PrintHPIDirectory(dir.Entries[EntryIndex].Directory,Tabs+1);
	}
    }
}

ShaderProgram UnitShader;
ShaderProgram OrthoShader;
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
unsigned char temp_bitmap[512*512];
stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;
void my_stbtt_initfont(void)
{
    char ttf_buffer[1<<20];
    FILE * file=fopen("data/times.ttf", "rb");
    if(!file)
    {
	LogError("Failed to load font file");
//	return;
    }
    fread(ttf_buffer, 1, 1<<20, file);
    stbtt_BakeFontBitmap((const unsigned char*)ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
    // can free ttf_buffer at this point
    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void my_stbtt_print(float x, float y, char *text)
{
    //TODO(Christof): actually make this work, 
    // assume orthographic projection with units = screen pixels, origin at top left
    glBindTexture(GL_TEXTURE_2D, ftex);
    glBegin(GL_QUADS);
    while (*text) {
	if (*text >= 32 && *text >0) {
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

struct ScreenText
{
    GLuint VertexArrayObject;
    GLuint FontTexture;
    int NumberOfVertices;
    float X,Y;
};

ScreenText SetupOnScreenText(char * Text, float X, float Y)
{
    int NumQuads=0;
    char * t=Text;
    while (*t) {
	if (*t >= 32 && *t >0) {
	    NumQuads++;
	}
	t++;
    }
    const int NUM_FLOATS_PER_QUAD=2*3*2*2;//2 triangles per quad, 3 verts per triangle, 2 position and 2 texture coords per vert
    GLfloat VertexAndTexCoordData[NumQuads*NUM_FLOATS_PER_QUAD];
    ScreenText result;
    result.X=X;
    result.Y=Y;
    result.FontTexture=ftex;
    result.NumberOfVertices=NumQuads * 6;
    glGenVertexArrays(1,&result.VertexArrayObject);
    for(int i=0;i<NumQuads;i++)
    {
	if (Text[i] >= 32 && Text[i] >0) {
	    stbtt_aligned_quad q;
	    //TODO(Christof): Make this position independant
	    stbtt_GetBakedQuad(cdata, 512,512, Text[i]-32, &X,&Y,&q,1);
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 0]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 1]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 2]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 3]=q.t0;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 4]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 5]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 6]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 7]=q.t1;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 8]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 9]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 10]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 11]=q.t1;


	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 12]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 13]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 14]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 15]=q.t1;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 16]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 17]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 18]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 19]=q.t0;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 20]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 21]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 22]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 23]=q.t0;
	}
    }
    glBindVertexArray(result.VertexArrayObject);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*NumQuads*NUM_FLOATS_PER_QUAD,VertexAndTexCoordData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,(GLvoid*)(sizeof(GLfloat)*2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    //glDeleteBuffers(1,&VertexBuffer);
    
    return result;
}

void RenderOnScreenText(ScreenText Text)
{
    glBindVertexArray(Text.VertexArrayObject);
    glBindTexture(GL_TEXTURE_2D,Text.FontTexture);
    glDrawArrays(GL_TRIANGLES,0,Text.NumberOfVertices);
}

ScreenText TestText;
int64_t StartTime=0;
int NumberOfFrames=0;


Object3d temp_model;
Matrix ProjectionMatrix;
Matrix ModelMatrix;
Matrix ViewMatrix;

GLuint ProjectionMatrixLocation;
GLuint ModelMatrixLocation;
GLuint ViewMatrixLocation;


int ModelIndex=138;
int FileIndex=0;
HPIEntry Models;
void LoadCurrentModel()
{
    if(!Models.IsDirectory)
    {
	LogWarning("No model files");
	return;
    }
    if(ModelIndex>=Models.Directory.NumberOfEntries)
	ModelIndex=Models.Directory.NumberOfEntries-1;
    if(ModelIndex<0)
	ModelIndex=0;

    char temp[Models.Directory.Entries[ModelIndex].File.FileSize];
    if(LoadHPIFileEntryData(Models.Directory.Entries[ModelIndex],temp))
    {
	if(temp_model.Name)
	    Unload3DO(&temp_model);
	Load3DOFromBuffer(temp,&temp_model);
	int size=snprintf(NULL, 0, "%d: %s (from %s)",ModelIndex,Models.Directory.Entries[ModelIndex].Name,Models.Directory.Entries[ModelIndex].ContainedInFile->Name)+1;
	char tmp[size];
	snprintf(tmp,size,"%d: %s (from %s)",ModelIndex,Models.Directory.Entries[ModelIndex].Name,Models.Directory.Entries[ModelIndex].ContainedInFile->Name);
	TestText=SetupOnScreenText(tmp,0,30);
	PrepareObject3dForRendering(&temp_model);
    }
}


void HandleKeyDown(SDL_Keysym key)
{
    switch(key.sym)
    {
    case SDLK_ESCAPE:
	quit=true;
	break;
    case SDLK_o:
	if(Models.IsDirectory)
	{
	    ModelIndex++;
	
	    if(ModelIndex>=Models.Directory.NumberOfEntries)
		ModelIndex--;
	    else
		LoadCurrentModel();
	}
	break;
    case SDLK_l:
	if(Models.IsDirectory)
	{
	ModelIndex--;
	if(ModelIndex<0)
	    ModelIndex=0;
	else
	    LoadCurrentModel();
	}
	break;
    }
}



void Setup()
{
#ifdef __WINDOWS__
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerformaceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    my_stbtt_initfont();

    SetProjectionMatrix(60,float(ScreenWidth)/ScreenHeight,1.0,100.0,&ProjectionMatrix);
    
    SetTranslationMatrix(0,0,-4,&ViewMatrix);
    SetRotationMatrix(1,0,0,0.5,&ModelMatrix);
    ViewMatrix = ViewMatrix*ModelMatrix;
    SetRotationMatrix(0,1,0,PI,&ModelMatrix);
    ViewMatrix = ViewMatrix*ModelMatrix;

    UnitShader=LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl");
    OrthoShader=LoadShaderProgram("shaders/ortho.vs.glsl","shaders/ortho.fs.glsl");
    glUseProgram(OrthoShader.ProgramID);
    glUniform1i(GetUniformLocation(OrthoShader,"UnitTexture"),0);
    
    glUseProgram(UnitShader.ProgramID);
    glUniform1i(GetUniformLocation(UnitShader,"UnitTexture"),0);

    ProjectionMatrixLocation = GetUniformLocation(UnitShader,"ProjectionMatrix");
    ModelMatrixLocation = GetUniformLocation(UnitShader,"ModelMatrix");
    ViewMatrixLocation = GetUniformLocation(UnitShader,"ViewMatrix");
    
    TestText=SetupOnScreenText("This is a test, now somewhat longer",0,30);
    //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CW);
    

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glUseProgram(OrthoShader.ProgramID);
    glUniform2iv(GetUniformLocation(OrthoShader,"Viewport"),1,viewport+2);
    LoadHPIFileCollection();
    LoadAllTextures();


    Models = FindEntryInAllFiles("objects3d");
    LoadCurrentModel();
    

    StartTime= GetTimeMillis();
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);                      // Turn Blending on
    glEnable(GL_DEPTH_TEST);        //Turn Depth Testing off
    glUseProgram(UnitShader.ProgramID);
    glBindTexture(GL_TEXTURE_2D,UnitTexture);
    UploadMatrix(&ProjectionMatrix,ProjectionMatrixLocation);


    SetRotationMatrix(0,1,0,PI/300,&ModelMatrix);
    ViewMatrix = ViewMatrix*ModelMatrix;

    UploadMatrix(&ViewMatrix,ViewMatrixLocation);
    RenderObject3d(&temp_model,0,ModelMatrixLocation);
    

    //TODO(Christof): Unit Rendering here


    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);                      // Turn Blending on
    glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off
    glUseProgram(OrthoShader.ProgramID);
    RenderOnScreenText(TestText);
    //my_stbtt_print(0,0,"Another Test");
    



    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed to render : %s",gluErrorString(ErrorValue));
    }
    NumberOfFrames++;
}

void Teardown()
{
    int64_t EndTime=GetTimeMillis();
    LogDebug("%d frames in %.3fs, %.2f FPS",NumberOfFrames,(EndTime-StartTime)/1000.0,NumberOfFrames/((EndTime-StartTime)/1000.0));
    UnloadShaderProgram(UnitShader);
    UnloadHPIFileCollection();
    Unload3DO(&temp_model);
}

bool32 SetupSDLWindow()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	LogError("SDL_Init Error: %s", SDL_GetError());
	return 0;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    MainSDLWindow = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    if (!MainSDLWindow)
    {
	LogError("SDL_CreateWindow Error: %s", SDL_GetError());
	return 0;
    }


    GLint GLMajorVer, GLMinorVer;
    
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed before create context : %s", gluErrorString(ErrorValue));
    }
    

    SDL_GLContext gContext = SDL_GL_CreateContext( MainSDLWindow);
    SDL_GL_MakeCurrent (MainSDLWindow,gContext);
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
	LogWarning("Warning: Unable to set VSync! SDL Error: %s",SDL_GetError());
    }

    glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVer);
    glGetIntegerv(GL_MINOR_VERSION, &GLMinorVer);
    LogDebug("OpenGL Version %d.%d",GLMajorVer,GLMinorVer);


    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed before glewInit : %s",gluErrorString(ErrorValue));
    }
  

    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
	LogError("Error initializing GLEW! %s", glewGetErrorString( glewError ));
	return 0;
    }

    
    //as we need to use glewExperimental - known issue (it segfaults otherwise!) - we encounter
    //another known issue, which is that while glewInit suceeds, it leaves opengl in an error state
    ErrorValue = glGetError();

    return 1;
}
    
