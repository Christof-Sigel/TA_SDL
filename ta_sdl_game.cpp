#include <stdio.h>

#include "platform_code.cpp"
#include "GL.cpp"
#include "UI.cpp"
#include "file_formats.cpp"
#include "ta_sdl_game.h"


struct GameState
{
    bool32 IsInitialised;
    MemoryArena GameArena;

    Matrix * ProjectionMatrix;
    Matrix * ModelMatrix;
    Matrix * ViewMatrix;

    int64_t StartTime=0;
    int NumberOfFrames=0;
    ShaderProgram UnitShader;
    UIElement TestElement[5];
    Object3d temp_model;
    GLuint ProjectionMatrixLocation;
    GLuint ModelMatrixLocation;
    GLuint ViewMatrixLocation;
    ShaderProgram MapShader;
    ShaderProgram FontShader;
    GLuint FontPositionLocation=-1,FontColorLocation=-1;
};


#include "ta_sdl_game_load.cpp"

void HandleKeyDown(SDL_Keysym key);
extern bool32 quit;
bool32 SetupSDLWindow();
void Render();
void Setup();
void Teardown();

void CheckResources();
void ReloadShaders();









int UnitIndex=0;


void LoadCurrentModel(GameState * CurrentGameState)
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
	if(CurrentGameState->temp_model.Name)
	    Unload3DO(&CurrentGameState->temp_model);
	Load3DOFromBuffer(temp,&CurrentGameState->temp_model);
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
	    CurrentGameState->TestElement[i]=SetupUIElementEnclosingText(X,Y, 0.25,0.75,0.25, 1,1,1, 5,(1.0-fabs(i-2.0)/4), 2,temp);
	    Y+=CurrentGameState->TestElement[i].Size.Height+5;
	}

	
	PrepareObject3dForRendering(&CurrentGameState->temp_model);
    }
}






const float DR=0.01,DG=0.02,DB=0.015;
float dr=DR,dg=DG,db=DB;

void HandleInput(InputState * Input, GameState * CurrentGameState)
{
    if(Input->KeyIsDown[SDLK_ESCAPE])
    {
	quit=true;
    }
    if(Input->KeyIsDown[SDLK_o] && !Input->KeyWasDown[SDLK_o])
    {
	UnitIndex--;;
	LoadCurrentModel(CurrentGameState);
    }
    if(Input->KeyIsDown[SDLK_l] && ! Input->KeyWasDown[SDLK_l])
    {
	UnitIndex++;
	LoadCurrentModel(CurrentGameState);
    }
    //HACK: FIX THIS SHIT
    if(Input->KeyIsDown[SDLK_UP&255])
    {
	CurrentGameState->ViewMatrix->Rotate(1,0,0,-0.01f);
    }
    if(Input->KeyIsDown[SDLK_DOWN&255])
    {
	CurrentGameState->ViewMatrix->Rotate(1,0,0,0.01f);
    }
    if(Input->KeyIsDown[SDLK_LEFT&255])
    {
	CurrentGameState->ViewMatrix->Rotate(0,1,0,-0.01f);
    }
    if(Input->KeyIsDown[SDLK_RIGHT&255])
    {
	CurrentGameState->ViewMatrix->Rotate(0,1,0,0.01f);
    }
    if(Input->KeyIsDown[SDLK_w])
    {
	CurrentGameState->ViewMatrix->Move(0,0,0.1);
    }
    if(Input->KeyIsDown[SDLK_s])
    {
	CurrentGameState->ViewMatrix->Move(0,0,-0.1);
    }
    if(Input->KeyIsDown[SDLK_a])
    {
	CurrentGameState->ViewMatrix->Move(0.1,0,0);
    }
    if(Input->KeyIsDown[SDLK_d])
    {
	CurrentGameState->ViewMatrix->Move(-0.1,0,0);
    }
    if(Input->KeyIsDown[SDLK_q])
    {
	CurrentGameState->ViewMatrix->Move(0,-0.1,0);
    }
    if(Input->KeyIsDown[SDLK_e])
    {
	CurrentGameState->ViewMatrix->Move(0,0.1,0);
    }

}


void SetupGameState( GameState * CurrentGameState)
{
    MemoryArena * GameArena = &CurrentGameState->GameArena;
    CurrentGameState->ProjectionMatrix = PushStruct(GameArena,Matrix);
    CurrentGameState->ViewMatrix = PushStruct(GameArena,Matrix);
    CurrentGameState->ModelMatrix = PushStruct(GameArena,Matrix);
    CurrentGameState->ProjectionMatrix->SetProjection(60,float(ScreenWidth)/ScreenHeight,1.0,1000.0);

    CurrentGameState->ViewMatrix->SetTranslation(0,0,-2);
    CurrentGameState->ViewMatrix->Rotate(1,0,0, 0.5);

        //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

}


void GameUpdateAndRender(InputState * Input, Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    if(!CurrentGameState->IsInitialised)
    {
	CurrentGameState->IsInitialised=1;
	InitializeArena(&CurrentGameState->GameArena,GameMemory->PermanentStoreSize-sizeof(GameState),GameMemory->PermanentStore+sizeof(GameState));
	SetupGameState(CurrentGameState);
    }
    HandleInput(Input,CurrentGameState);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(CurrentGameState->UnitShader.ProgramID);
    glBindTexture(GL_TEXTURE_2D,UnitTexture);
    CurrentGameState->ProjectionMatrix->Upload(CurrentGameState->ProjectionMatrixLocation);


    //CurrentGameState->ViewMatrix->Rotate(0,1,0, PI/300);
    //CurrentGameState->ViewMatrix->Move(0.01,-0.01,-0.01);
    
    CurrentGameState->ViewMatrix->Upload(CurrentGameState->ViewMatrixLocation);
    RenderObject3d(&CurrentGameState->temp_model,0,CurrentGameState->ModelMatrixLocation);

    glUseProgram(CurrentGameState->MapShader.ProgramID);
    CurrentGameState->ProjectionMatrix->Upload(GetUniformLocation(CurrentGameState->MapShader,"Projection"));
    CurrentGameState->ViewMatrix->Upload(GetUniformLocation(CurrentGameState->MapShader,"View"));
    TestMap.Render(&CurrentGameState->MapShader);
    

    //TODO(Christof): Unit Rendering here


    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);                      // Turn Blending on
    glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off

    for(int i=0;i<5;i++)
	RenderUIElement(CurrentGameState->TestElement[i],&CurrentGameState->FontShader,CurrentGameState->FontPositionLocation,CurrentGameState->FontColorLocation);


    
    CurrentGameState->NumberOfFrames++;
}

