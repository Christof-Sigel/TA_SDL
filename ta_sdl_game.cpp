#include <stdio.h>
#include <SDL2/SDL.h>
#ifdef __LINUX__
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#else
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
typedef int32_t bool32;
#include <GL/glew.h>
#include "ta_sdl_game.h"


#include "platform_code.cpp"
#include "GL.cpp"
#include "UI.cpp"
#include "file_formats.cpp"





#include "ta_sdl_game_load.cpp"

void HandleKeyDown(SDL_Keysym key);
bool32 SetupSDLWindow();
void Render();
void Setup();
void Teardown();

void CheckResources();
void ReloadShaders();












void LoadCurrentModel(GameState * CurrentGameState)
{
    if(CurrentGameState->UnitIndex>(int)CurrentGameState->Units.size()-1)
	CurrentGameState->UnitIndex=0;
    if(CurrentGameState->UnitIndex<0)
	CurrentGameState->UnitIndex=CurrentGameState->Units.size()-1;

    char * UnitName=CurrentGameState->Units[CurrentGameState->UnitIndex].GetString("UnitName");
    int len=snprintf(0,0,"objects3d/%s.3do",UnitName)+1;
    char ModelName[len];
    snprintf(ModelName,len,"objects3d/%s.3do",UnitName);
    
    HPIEntry Entry=FindEntryInAllFiles(ModelName,CurrentGameState); 
    uint8_t temp[Entry.File.FileSize];
    if(LoadHPIFileEntryData(Entry,temp))
    {
	if(CurrentGameState->temp_model->Name)
	    Unload3DO(CurrentGameState->temp_model);
	Load3DOFromBuffer(temp,CurrentGameState->temp_model,CurrentGameState->NextTexture,CurrentGameState->Textures);
	//TODO(Christof): free memory correctly
	float X=0,Y=0;
	for(int i=0;i<5;i++)
	{
	    ScreenText * NameText=(ScreenText*)malloc(sizeof(ScreenText));
	    ScreenText * DescText=(ScreenText*)malloc(sizeof(ScreenText));
	    int Index=CurrentGameState->UnitIndex+(i-2);
	    if(Index<0)
		Index+=CurrentGameState->Units.size();
	    if(Index>=CurrentGameState->Units.size())
		Index-=CurrentGameState->Units.size();
	    char * Name=CurrentGameState->Units[Index].GetString("Name");
	    char * SideName;
	    UnitSide Side=CurrentGameState->Units[Index].GetSide();
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
		char * Desc=CurrentGameState->Units[Index].GetString("Description");
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

	
	PrepareObject3dForRendering(CurrentGameState->temp_model,CurrentGameState->PaletteData);
    }
}

void HandleInput(InputState * Input, GameState * CurrentGameState)
{
    if(Input->KeyIsDown[SDLK_ESCAPE])
    {
	CurrentGameState->Quit=true;
    }
    if(Input->KeyIsDown[SDLK_o] && !Input->KeyWasDown[SDLK_o])
    {
	CurrentGameState->UnitIndex--;;
	LoadCurrentModel(CurrentGameState);
    }
    if(Input->KeyIsDown[SDLK_l] && !Input->KeyWasDown[SDLK_l])
    {
	CurrentGameState->UnitIndex++;
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

    CurrentGameState->MapShader = PushStruct(GameArena, ShaderProgram);
    CurrentGameState->UnitShader = PushStruct(GameArena, ShaderProgram);
    CurrentGameState->FontShader = PushStruct(GameArena, ShaderProgram);
    CurrentGameState->UIElementShaderProgram = PushStruct(GameArena, ShaderProgram);

    CurrentGameState->temp_model = PushStruct(GameArena, Object3d);
    CurrentGameState->TestMap = PushStruct(GameArena, TAMap);
    CurrentGameState->GlobalArchiveCollection = PushStruct(GameArena,HPIFileCollection);
    CurrentGameState->TestElement = PushArray(GameArena,5,UIElement);
    
    CurrentGameState->ProjectionMatrix->SetProjection(60,float(CurrentGameState->ScreenWidth)/CurrentGameState->ScreenHeight,1.0,1000.0);

    CurrentGameState->ViewMatrix->SetTranslation(0,0,-2);
    CurrentGameState->ViewMatrix->Rotate(1,0,0, 0.5);


    CurrentGameState->Textures = PushArray(GameArena,MAX_NUMBER_OF_TEXTURE,Texture);
    CurrentGameState->TextureData = PushArray(GameArena,TEXTURE_HEIGHT*TEXTURE_WIDTH*4,uint8_t);
    CurrentGameState->PaletteData = PushArray(GameArena,1024,uint8_t);
    CurrentGameState->FontBitmap = PushArray(GameArena,FONT_BITMAP_SIZE*FONT_BITMAP_SIZE,uint8_t);

        //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void InitialiseGame(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    CurrentGameState->IsInitialised=1;
    InitializeArena(&CurrentGameState->GameArena,GameMemory->PermanentStoreSize-sizeof(GameState),GameMemory->PermanentStore+sizeof(GameState));
    InitializeArena(&CurrentGameState->TempArena,GameMemory->TransientStoreSize-sizeof(GameState),GameMemory->TransientStore);
    SetupGameState(CurrentGameState);
}

void GameUpdateAndRender(InputState * Input, Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    if(!CurrentGameState->IsInitialised)
    {
	InitialiseGame(GameMemory);
    }
    
    HandleInput(Input,CurrentGameState);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(CurrentGameState->UnitShader->ProgramID);
    glBindTexture(GL_TEXTURE_2D,CurrentGameState->UnitTexture);
    CurrentGameState->ProjectionMatrix->Upload(CurrentGameState->ProjectionMatrixLocation);


    //CurrentGameState->ViewMatrix->Rotate(0,1,0, PI/300);
    //CurrentGameState->ViewMatrix->Move(0.01,-0.01,-0.01);
    
    CurrentGameState->ViewMatrix->Upload(CurrentGameState->ViewMatrixLocation);
    RenderObject3d(CurrentGameState->temp_model,0,CurrentGameState->ModelMatrixLocation);

    glUseProgram(CurrentGameState->MapShader->ProgramID);
    CurrentGameState->ProjectionMatrix->Upload(GetUniformLocation(CurrentGameState->MapShader,"Projection"));
    CurrentGameState->ViewMatrix->Upload(GetUniformLocation(CurrentGameState->MapShader,"View"));
    CurrentGameState->TestMap->Render(CurrentGameState->MapShader);
    

    //TODO(Christof): Unit Rendering here


    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);                      // Turn Blending on
    glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off

    for(int i=0;i<5;i++)
	RenderUIElement(CurrentGameState->TestElement[i],CurrentGameState->UIElementShaderProgram,CurrentGameState->UIElementPositionLocation, CurrentGameState->UIElementSizeLocation,  CurrentGameState->UIElementColorLocation, CurrentGameState->UIElementBorderColorLocation, CurrentGameState->UIElementBorderWidthLocation,  CurrentGameState->UIElementAlphaLocation,  CurrentGameState->UIElementRenderingVertexBuffer, CurrentGameState->FontShader,  CurrentGameState->FontPositionLocation,  CurrentGameState->FontColorLocation);


    
    CurrentGameState->NumberOfFrames++;
}

