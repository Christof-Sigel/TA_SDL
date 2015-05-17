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


void LoadCurrentModel(GameState * CurrentGameState)
{
    if(CurrentGameState->UnitIndex>(int)CurrentGameState->Units.Size-1)
	CurrentGameState->UnitIndex=0;
    if(CurrentGameState->UnitIndex<0)
	CurrentGameState->UnitIndex=CurrentGameState->Units.Size-1;

    char * UnitName=CurrentGameState->Units.Details[CurrentGameState->UnitIndex].GetString("UnitName");
    int len=snprintf(0,0,"objects3d/%s.3do",UnitName)+1;
    //char ModelName[len];
    STACK_ARRAY(ModelName,len,char);
    snprintf(ModelName,len,"objects3d/%s.3do",UnitName);
    
    HPIEntry Entry=FindEntryInAllFiles(ModelName,CurrentGameState); 
    //uint8_t temp[Entry.File.FileSize];
    STACK_ARRAY(temp,Entry.File.FileSize,uint8_t);
    if(LoadHPIFileEntryData(Entry,temp))
    {
	int ScriptLength=snprintf(0,0,"scripts/%s.cob",UnitName)+1;
	//char ScriptName[ScriptLength];
	STACK_ARRAY(ScriptName,ScriptLength,char);
	snprintf(ScriptName,ScriptLength,"scripts/%s.cob",UnitName);
	HPIEntry ScriptEntry=FindEntryInAllFiles(ScriptName,CurrentGameState); 
	//uint8_t ScriptBuffer[ScriptEntry.File.FileSize];
	STACK_ARRAY(ScriptBuffer, ScriptEntry.File.FileSize, uint8_t);
	if(LoadHPIFileEntryData(ScriptEntry,ScriptBuffer))
	{
	    LoadUnitScriptFromBuffer(CurrentGameState->CurrentUnitScript, ScriptBuffer,ScriptEntry.File.FileSize,&CurrentGameState->GameArena);
    
	    if(CurrentGameState->temp_model->Vertices)
		Unload3DO(CurrentGameState->temp_model);
	    Load3DOFromBuffer(temp,CurrentGameState->temp_model,CurrentGameState->NextTexture,CurrentGameState->Textures,&CurrentGameState->GameArena);
	    //TODO(Christof): free memory correctly
	    float X=0,Y=0;
	    for(int i=0;i<5;i++)
	    {
		int Index=CurrentGameState->UnitIndex+(i-2);
		if(Index<0)
		    Index+=CurrentGameState->Units.Size;
		if(Index>=CurrentGameState->Units.Size)
		    Index-=CurrentGameState->Units.Size;
		char * Name=CurrentGameState->Units.Details[Index].GetString("Name");
		const char * SideName;
		UnitSide Side=CurrentGameState->Units.Details[Index].GetSide();
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
		//char tmp[size];
		STACK_ARRAY(tmp,size,char);
		snprintf(tmp,size,"%s: %s",SideName,Name);
		CurrentGameState->NameAndDescText[i*2+0]=SetupOnScreenText(tmp,10,30, 1,1,1, CurrentGameState->Times32);

		{
		    char * Desc=CurrentGameState->Units.Details[Index].GetString("Description");
		    int size=snprintf(NULL, 0, "%s",Desc)+1;
		    //char tmp[size];
		    STACK_ARRAY(tmp,size,char);
		    snprintf(tmp,size,"%s",Desc);
		    CurrentGameState->NameAndDescText[i*2+1]=SetupOnScreenText(tmp,15,54, 1,1,1, CurrentGameState->Times24);
		}
		CurrentGameState->TestElement[i]=SetupUIElementEnclosingText(X,Y, 0.25f,0.75f,0.25f, 1,1,1, 5,(float)(1.0-fabs(i-2.0)/4), 2,&CurrentGameState->NameAndDescText[i*2]);
		Y+=CurrentGameState->TestElement[i].Size.Height+5;
	    }
	
	    UnitDetails * CurrentUnit = & CurrentGameState->Units.Details[CurrentGameState->UnitIndex];
	    for(int i=0;i<NUMBER_OF_UNIT_DETAILS && i<CurrentUnit->DetailsSize;i++)
	    {
		int size = snprintf(NULL,0,"%d) %s : %s",i+1,CurrentUnit->Details[i].Name,CurrentUnit->Details[i].Value)+1;
		//char temp[size];
		STACK_ARRAY(temp,size,char);

		snprintf(temp,size,"%d) %s : %s",i+1,CurrentUnit->Details[i].Name,CurrentUnit->Details[i].Value);

		CurrentGameState->UnitDetailsText[i] = SetupOnScreenText(temp, (float)CurrentGameState->ScreenWidth-350, (float)(i*(CurrentGameState->Times24->Height+5)+20),  0,0.75f,0, CurrentGameState->Times24);
	    }
	
	    PrepareObject3dForRendering(CurrentGameState->temp_model,CurrentGameState->PaletteData);
	}
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


    
    CurrentGameState->ViewMatrix->SetTranslation(1,-2.5,2);
    CurrentGameState->ViewMatrix->Rotate(0,1,0, (float)PI*0.75f);


    CurrentGameState->Textures = PushArray(GameArena,MAX_NUMBER_OF_TEXTURE,Texture);
    CurrentGameState->TextureData = PushArray(GameArena,TEXTURE_HEIGHT*TEXTURE_WIDTH*4,uint8_t);
    CurrentGameState->PaletteData = PushArray(GameArena,1024,uint8_t);
    CurrentGameState->FontBitmap = PushArray(GameArena,FONT_BITMAP_SIZE*FONT_BITMAP_SIZE,uint8_t);
    CurrentGameState->Units.Details = PushArray(GameArena,MAX_UNITS_LOADED,UnitDetails);

    CurrentGameState->Times32 = PushStruct(GameArena,FontDetails);
    CurrentGameState->Times24 = PushStruct(GameArena,FontDetails);
    CurrentGameState->Times16 = PushStruct(GameArena,FontDetails);

    CurrentGameState->NameAndDescText = PushArray(GameArena,5*2,ScreenText);
    
    CurrentGameState->UnitDetailsText = PushArray(GameArena,NUMBER_OF_UNIT_DETAILS,ScreenText);
    CurrentGameState->CurrentUnitScript = PushStruct(GameArena, UnitScript);
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

extern "C"
{
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
	Matrix ModelMatrix;
	ModelMatrix.SetTranslation(0.5,1.4,0.5);
	RenderObject3d(CurrentGameState->temp_model,0,CurrentGameState->ModelMatrixLocation,ModelMatrix);

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


	for(int i=0;i<NUMBER_OF_UNIT_DETAILS;i++)
	    RenderOnScreenText(CurrentGameState->UnitDetailsText[i],CurrentGameState->FontShader,CurrentGameState->FontPositionLocation,CurrentGameState->FontColorLocation);
    
	CurrentGameState->NumberOfFrames++;
    }

}
