#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>

typedef int32_t bool32;


#include "platform_code.cpp"
#include "GL.cpp"
#include "UI.cpp"
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



ScreenText TestText;
ScreenText TestText2;
UIElement TestElement;
int64_t StartTime=0;
int NumberOfFrames=0;


Object3d temp_model;
Matrix ProjectionMatrix;
Matrix ModelMatrix;
Matrix ViewMatrix;

GLuint ProjectionMatrixLocation;
GLuint ModelMatrixLocation;
GLuint ViewMatrixLocation;


int UnitIndex=0;
std::vector<UnitDetails> Units;

void LoadCurrentModel()
{
    if(UnitIndex>Units.size()-1)
	UnitIndex=Units.size()-1;
    if(UnitIndex<0)
	UnitIndex=0;

    char * UnitName=Units[UnitIndex].GetString("UnitName");
    int len=snprintf(0,0,"objects3d/%s.3do",UnitName)+1;
    char ModelName[len];
    snprintf(ModelName,len,"objects3d/%s.3do",UnitName);
    
    HPIEntry Entry=FindEntryInAllFiles(ModelName);
    
    char temp[Entry.File.FileSize];
    if(LoadHPIFileEntryData(Entry,temp))
    {
	if(temp_model.Name)
	    Unload3DO(&temp_model);
	Load3DOFromBuffer(temp,&temp_model);
	char * Name=Units[UnitIndex].GetString("Name");
	char * SideName;
	UnitSide Side=Units[UnitIndex].GetSide();
	switch(Side)
	{
	case SIDE_ARM:
	    SideName = "ARM";
	    break;
	case SIDE_CORE:
	    SideName = "CORE";
	    break;
	default:
	    SideName="UNKNOWN";
	    break;
	}

	int size=snprintf(NULL, 0, "%s - %s (from %s)",SideName,Name,Entry.ContainedInFile->Name)+1;
	char tmp[size];
	snprintf(tmp,size,"%s - %s (from %s)",SideName,Name,Entry.ContainedInFile->Name);
	TestText=SetupOnScreenText(tmp,10,30, 1,1,1, &Times32);

	{
	    char * Desc=Units[UnitIndex].GetString("Description");
	int size=snprintf(NULL, 0, "%s",Desc)+1;
	char tmp[size];
	snprintf(tmp,size,"%s",Desc);
	TestText2=SetupOnScreenText(tmp,15,54, 1,1,1, &Times24);
	}
	ScreenText ** temp=(ScreenText**)malloc(sizeof(ScreenText*)*2);
	temp[0]=&TempText1;
	temp[1]=&TempText2;
	SetupUIElementEnclosingText(0,0, 0.25,0.75,0.25, 1,1,1, 5,1, 2,temp); 

	
	PrepareObject3dForRendering(&temp_model);
    }
}


void HandleKeyDown(SDL_Keysym key)
{
    switch(key.sym)
    {
    case SDLK_q:
    case SDLK_ESCAPE:
	quit=true;
	break;
    case SDLK_o:
	UnitIndex++;
	LoadCurrentModel();
	break;
    case SDLK_l:
	UnitIndex--;
	LoadCurrentModel();
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
    SetupTextRendering();
    SetupUIElementRender();
    
    ProjectionMatrix.SetProjection(60,float(ScreenWidth)/ScreenHeight,1.0,100.0);

    ViewMatrix.SetTranslation(0,0,-4);
    ViewMatrix.Rotate(1,0,0, 0.5);
    ViewMatrix.Rotate(0,1,0, PI);
    
    UnitShader=LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl");
    
    glUseProgram(UnitShader.ProgramID);
    glUniform1i(GetUniformLocation(UnitShader,"UnitTexture"),0);

    ProjectionMatrixLocation = GetUniformLocation(UnitShader,"ProjectionMatrix");
    ModelMatrixLocation = GetUniformLocation(UnitShader,"ModelMatrix");
    ViewMatrixLocation = GetUniformLocation(UnitShader,"ViewMatrix");


    TestElement=SetupUIElement(0,0, 600,100, 1,1,1, 0,1,0, 5,0.5, 0);
    
    TestText2=SetupOnScreenText("This is a test, now somewhat longer",0,30, 1,1,1, &Times32);

    
    //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CW);
        
    LoadHPIFileCollection();
    LoadAllTextures();


    HPIEntry Entry=FindEntryInAllFiles("units");
    if(Entry.IsDirectory)
    {
	for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
	{
	    char temp[Entry.Directory.Entries[i].File.FileSize];
	    if(LoadHPIFileEntryData(Entry.Directory.Entries[i],temp))
	    {
		UnitDetails deets;
		if(strstr(Entry.Directory.Entries[i].Name,".FBI"))
		{
		    LoadFBIFileFromBuffer(&deets,temp);
		    Units.push_back(deets);
		}
	    }
	}
    }
    UnloadCompositeEntry(&Entry);

    LoadCurrentModel();
   
    
    


    StartTime= GetTimeMillis();
}

const float DR=0.01,DG=0.02,DB=0.015;
float dr=DR,dg=DG,db=DB;

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);                      // Turn Blending on
    glEnable(GL_DEPTH_TEST);        //Turn Depth Testing off
    glUseProgram(UnitShader.ProgramID);
    glBindTexture(GL_TEXTURE_2D,UnitTexture);
    ProjectionMatrix.Upload(ProjectionMatrixLocation);


    ViewMatrix.Rotate(0,1,0, PI/300);
    // ViewMatrix.Move(0.01,0,0);
    
    ViewMatrix.Upload(ViewMatrixLocation);
    RenderObject3d(&temp_model,0,ModelMatrixLocation);
    

    //TODO(Christof): Unit Rendering here


    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);                      // Turn Blending on
    glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off

    RenderUIElement(TestElement);


    // RenderOnScreenText(TestText);
    //RenderOnScreenText(TestText2);
    //my_stbtt_print(0,0,"Another Test");
    return;
    TestText.Color.Red += dr;
    if(TestText.Color.Red > 1.0)
	dr=-DR;
    if(TestText.Color.Red<0.0)
	dr=DR;
    
    TestText.Color.Green += dg;
    if(TestText.Color.Green > 1.0)
	dg=-DG;
    if(TestText.Color.Green<0.0)
	dg=DG;
    
    TestText.Color.Blue += db;
    if(TestText.Color.Blue > 1.0)
	db=-DB;
    if(TestText.Color.Blue<0.0)
	db=DB;


    
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
    
