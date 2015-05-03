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
void CheckResources();
void ReloadShaders();

int ScreenWidth=1024,ScreenHeight=768;

int main(int argc, char * argv[])
{
    if(!SetupSDLWindow())
	return 1;

    Setup();
    
    SDL_Event e;
    while( !quit )
    {
	CheckResources();
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



UIElement TestElement[5];
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
    if(UnitIndex>(int)Units.size()-1)
	UnitIndex=0;
    if(UnitIndex<0)
	UnitIndex=Units.size()-1;

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
	//TODO(Christof): free memory correctly
	float X=0,Y=0;
	for(int i=0;i<5;i++)
	{
	    ScreenText * NameText=(ScreenText*)malloc(sizeof(ScreenText));
	    ScreenText * DescText=(ScreenText*)malloc(sizeof(ScreenText));
	    int Index=UnitIndex+(i-2);
	    if(Index<0)
		Index+=Units.size();
	    if(Index>=Units.size())
		Index-=Units.size();
	    char * Name=Units[Index].GetString("Name");
	    char * SideName;
	    UnitSide Side=Units[Index].GetSide();
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

	    int size=snprintf(NULL, 0, "%s: %s",SideName,Name)+1;
	    char tmp[size];
	    snprintf(tmp,size,"%s: %s",SideName,Name);
	    *NameText=SetupOnScreenText(tmp,10,30, 1,1,1, &Times32);

	    {
		char * Desc=Units[Index].GetString("Description");
		int size=snprintf(NULL, 0, "%s",Desc)+1;
		char tmp[size];
		snprintf(tmp,size,"%s",Desc);
		*DescText=SetupOnScreenText(tmp,15,54, 1,1,1, &Times24);
	    }
	    ScreenText ** temp=(ScreenText**)malloc(sizeof(ScreenText*)*2);
	    temp[0]=NameText;
	    temp[1]=DescText;
	    TestElement[i]=SetupUIElementEnclosingText(X,Y, 0.25,0.75,0.25, 1,1,1, 5,(1.0-fabs(i-2.0)/4), 2,temp);
	    Y+=TestElement[i].Size.Height+5;
	}

	
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
	UnitIndex--;;
	LoadCurrentModel();
	break;
    case SDLK_l:
	UnitIndex++;
	LoadCurrentModel();
	break;
    case SDLK_UP:
	ViewMatrix.Rotate(1,0,0,-0.01f);
	break;
    case SDLK_DOWN:
	ViewMatrix.Rotate(1,0,0,0.01f);
	break;
    case SDLK_LEFT:
	ViewMatrix.Rotate(0,1,0,-0.01f);
	break;
    case SDLK_RIGHT:
	ViewMatrix.Rotate(0,1,0,0.01f);
	break;
    case SDLK_w:
	ViewMatrix.Move(0,0,0.1);
	break;
    case SDLK_s:
	ViewMatrix.Move(0,0,-0.1);
	break;	
    case SDLK_a:
	ViewMatrix.Move(-0.1,0,0);
	break;
    case SDLK_d:
	ViewMatrix.Move(0.1,0,0);
	break;
    case SDLK_q:
	ViewMatrix.Move(0,-0.1,0);
	break;
    case SDLK_e:
	ViewMatrix.Move(0,0.1,0);
	break;
	
    }
}


TAMap TestMap={0};
void Setup()
{
#ifdef __WINDOWS__
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerformaceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    LoadFonts();
    SetupUIElementRender();
    
    ProjectionMatrix.SetProjection(60,float(ScreenWidth)/ScreenHeight,1.0,1000.0);

    ViewMatrix.SetTranslation(0,0,-2);
    ViewMatrix.Rotate(1,0,0, 0.5);
    // ViewMatrix.Rotate(0,1,0, PI);
    //ViewMatrix.Rotate(0,1,0, -PI/4);
    //ViewMatrix.Move(1,0,0);
    
    ReloadShaders();

    //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
        
    LoadHPIFileCollection();
    LoadAllTextures();
    HPIEntry Map = FindEntryInAllFiles("maps/Greenhaven.tnt");
    if(Map.Name)
    {
	char * temp = (char*)malloc(Map.File.FileSize);
    
	if(LoadHPIFileEntryData(Map,temp))
	{
	    TestMap=LoadTNTFromBuffer(temp);
	}
	else
	    LogDebug("failed to load map buffer from hpi");
	free(temp);
    }
    else
	LogDebug("failed to load map");
    
	

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


    //ViewMatrix.Rotate(0,1,0, PI/300);
    //ViewMatrix.Move(0.01,-0.01,-0.01);
    
    ViewMatrix.Upload(ViewMatrixLocation);
    RenderObject3d(&temp_model,0,ModelMatrixLocation);

    glUseProgram(MapShader.ProgramID);
    ProjectionMatrix.Upload(GetUniformLocation(MapShader,"Projection"));
    ViewMatrix.Upload(GetUniformLocation(MapShader,"View"));
    TestMap.Render();
    

    //TODO(Christof): Unit Rendering here


    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);                      // Turn Blending on
    glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off

    for(int i=0;i<5;i++)
    RenderUIElement(TestElement[i]);


    
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

void ReloadShaders()
{
    if(UnitShader.ProgramID)
	UnloadShaderProgram(UnitShader);
    if(MapShader.ProgramID)
	UnloadShaderProgram(MapShader);
    UnitShader=LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl");
    
    glUseProgram(UnitShader.ProgramID);
    glUniform1i(GetUniformLocation(UnitShader,"UnitTexture"),0);

    MapShader=LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl");
    
    glUseProgram(MapShader.ProgramID);
    glUniform1i(GetUniformLocation(MapShader,"Texture"),0);

    ProjectionMatrixLocation = GetUniformLocation(UnitShader,"ProjectionMatrix");
    ModelMatrixLocation = GetUniformLocation(UnitShader,"ModelMatrix");
    ViewMatrixLocation = GetUniformLocation(UnitShader,"ViewMatrix");


    FontShader=LoadShaderProgram("shaders/font.vs.glsl","shaders/font.fs.glsl");
    glUseProgram(FontShader.ProgramID);
    glUniform1i(GetUniformLocation(FontShader,"Texture"),0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glUniform2iv(GetUniformLocation(FontShader,"Viewport"),1,viewport+2);
    FontPositionLocation=GetUniformLocation(FontShader,"Position");
    FontColorLocation=GetUniformLocation(FontShader,"TextColor");

    
    UIElementShaderProgram=LoadShaderProgram("shaders/UI.vs.glsl","shaders/UI.fs.glsl");
    glUseProgram(UIElementShaderProgram.ProgramID);

    UIElementPositionLocation = GetUniformLocation(UIElementShaderProgram,"Position");
    UIElementSizeLocation = GetUniformLocation(UIElementShaderProgram,"Size");
    UIElementColorLocation = GetUniformLocation(UIElementShaderProgram,"Color");
    UIElementBorderColorLocation = GetUniformLocation(UIElementShaderProgram,"BorderColor");
    UIElementBorderWidthLocation = GetUniformLocation(UIElementShaderProgram,"BorderWidth");
    UIElementAlphaLocation = GetUniformLocation(UIElementShaderProgram,"Alpha");

    glUniform2iv(GetUniformLocation(UIElementShaderProgram,"Viewport"),1,viewport+2);

}

void CheckResources()
{
    uint64_t UnitVertexShaderTime = GetFileModifiedTime("shaders/unit3do.vs.glsl");
    uint64_t UnitPixelShaderTime = GetFileModifiedTime("shaders/unit3do.fs.glsl");
    uint64_t MapVertexShaderTime = GetFileModifiedTime("shaders/map.vs.glsl");
    uint64_t MapPixelShaderTime = GetFileModifiedTime("shaders/map.fs.glsl");

    uint64_t UIVertexShaderTime = GetFileModifiedTime("shaders/UI.vs.glsl");
    uint64_t UIPixelShaderTime = GetFileModifiedTime("shaders/UI.fs.glsl");

    uint64_t TextVertexShaderTime = GetFileModifiedTime("shaders/font.vs.glsl");
    uint64_t TextPixelShaderTime = GetFileModifiedTime("shaders/font.fs.glsl");
    if(UnitVertexShaderTime > UnitShader.VertexFileModifiedTime || UnitPixelShaderTime > UnitShader.PixelFileModifiedTime
       || MapPixelShaderTime > MapShader.VertexFileModifiedTime || MapVertexShaderTime > MapShader.PixelFileModifiedTime
       || UIVertexShaderTime > UIElementShaderProgram.VertexFileModifiedTime || UIPixelShaderTime > UIElementShaderProgram.PixelFileModifiedTime
       || TextVertexShaderTime > FontShader.VertexFileModifiedTime || TextPixelShaderTime > FontShader.PixelFileModifiedTime
	)
    ReloadShaders();
}
    
