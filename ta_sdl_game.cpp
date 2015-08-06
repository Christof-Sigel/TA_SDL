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

#include <GL/glew.h>
#include "ta_sdl_game.h"


#include "GL.cpp"
#include "file_formats.cpp"


#include "ta_sdl_game_load.cpp"


void LoadCurrentModel(GameState * CurrentGameState)
{
    if(CurrentGameState->UnitIndex>(int)CurrentGameState->Units.Size-1)
	CurrentGameState->UnitIndex=0;
    if(CurrentGameState->UnitIndex<0)
	CurrentGameState->UnitIndex=CurrentGameState->Units.Size-1;

    char * UnitName=CurrentGameState->Units.Details[CurrentGameState->UnitIndex].GetString("UnitName");
    const int MAX_MODEL_NAME=64;
    char Name[MAX_MODEL_NAME];
    snprintf(Name,MAX_MODEL_NAME,"objects3d/%s.3do",UnitName);
    
    HPIEntry Entry=FindEntryInAllFiles(Name,&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
    //u8 temp[Entry.File.FileSize];
    u8 * temp = PushArray(&CurrentGameState->TempArena,Entry.File.FileSize,u8 );
    if(LoadHPIFileEntryData(Entry,temp,&CurrentGameState->TempArena))
    {
	snprintf(Name,MAX_MODEL_NAME,"scripts/%s.cob",UnitName);
	HPIEntry ScriptEntry=FindEntryInAllFiles(Name,&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
	//u8 ScriptBuffer[ScriptEntry.File.FileSize];
	u8 * ScriptBuffer = PushArray(&CurrentGameState->TempArena, ScriptEntry.File.FileSize, u8 );
	if(LoadHPIFileEntryData(ScriptEntry,ScriptBuffer,&CurrentGameState->TempArena))
	{
	    LoadUnitScriptFromBuffer(&CurrentGameState->CurrentUnitScript, ScriptBuffer,&CurrentGameState->GameArena);
	    CurrentGameState->CurrentScriptPool={};
	    

	    if(CurrentGameState->temp_model.Vertices)
		Unload3DO(&CurrentGameState->temp_model);
	    Load3DOFromBuffer(temp,&CurrentGameState->temp_model,&CurrentGameState->UnitTextures,&CurrentGameState->GameArena);
	    InitTransformationDetails(&CurrentGameState->temp_model, &CurrentGameState->UnitTransformationDetails, &CurrentGameState->GameArena);
	    StartNewEntryPoint(&CurrentGameState->CurrentScriptPool, &CurrentGameState->CurrentUnitScript, "Create",0, 0, &CurrentGameState->UnitTransformationDetails);
	    s32 Args[] ={1*COB_ANGULAR_CONSTANT};
	    StartNewEntryPoint(&CurrentGameState->CurrentScriptPool, &CurrentGameState->CurrentUnitScript, "Activate",1, Args, &CurrentGameState->UnitTransformationDetails);
	    StartNewEntryPoint(&CurrentGameState->CurrentScriptPool, &CurrentGameState->CurrentUnitScript, "StartBuilding",1, Args, &CurrentGameState->UnitTransformationDetails);

	    s32 FireArgs[] ={5*COB_ANGULAR_CONSTANT, -0.25*COB_ANGULAR_CONSTANT};
	    StartNewEntryPoint(&CurrentGameState->CurrentScriptPool, &CurrentGameState->CurrentUnitScript, "AimPrimary",2, FireArgs, &CurrentGameState->UnitTransformationDetails);

//	    PrepareObject3dForRendering(CurrentGameState->temp_model,CurrentGameState->PaletteData);
	}
	PopArray(&CurrentGameState->TempArena, ScriptBuffer,ScriptEntry.File.FileSize, u8 );

    }
    PopArray(&CurrentGameState->TempArena, temp,Entry.File.FileSize,u8 );
}

static s32 Side=0;

Matrix FPSViewMatrix(float * eye, float pitch, float yaw);
void HandleInput(InputState * Input, GameState * CurrentGameState)
{
    Matrix ViewRotation = FPSViewMatrix(CurrentGameState->CameraTranslation, CurrentGameState->CameraXRotation, CurrentGameState->CameraYRotation);
//    ViewRotation.Rotate(1,0,0, CurrentGameState->CameraXRotation);
    //  ViewRotation.Rotate(0,1,0, CurrentGameState->CameraYRotation);
    const float CameraTranslation = 1.0;
    float DX[3] = { ViewRotation.Contents[0*4+0] * CameraTranslation + ViewRotation.Contents[3*4+0],
		    ViewRotation.Contents[0*4+1] * CameraTranslation + ViewRotation.Contents[3*4+1],
		    ViewRotation.Contents[0*4+2] * CameraTranslation + ViewRotation.Contents[3*4+2]};

    float DY[3] = { ViewRotation.Contents[1*4+0] * CameraTranslation + ViewRotation.Contents[3*4+0],
		    ViewRotation.Contents[1*4+1] * CameraTranslation + ViewRotation.Contents[3*4+1],
		    ViewRotation.Contents[1*4+2] * CameraTranslation + ViewRotation.Contents[3*4+2]};

    float DZ[3] = { ViewRotation.Contents[2*4+0] * CameraTranslation + ViewRotation.Contents[3*4+0],
		    ViewRotation.Contents[2*4+1] * CameraTranslation + ViewRotation.Contents[3*4+1],
		    ViewRotation.Contents[2*4+2] * CameraTranslation + ViewRotation.Contents[3*4+2]};

    for(int i=SDLK_0;i<=SDLK_9;i++)
    {
	if(Input->KeyIsDown[i])
	{
	    int ScriptNumber = i -SDLK_0;
	    if(ScriptNumber < CurrentGameState->CurrentUnitScript.NumberOfFunctions)
	    {

		CurrentGameState->CurrentScriptPool.Scripts[CurrentGameState->CurrentScriptPool.NumberOfScripts].ScriptNumber = ScriptNumber;
		CurrentGameState->CurrentScriptPool.Scripts[CurrentGameState->CurrentScriptPool.NumberOfScripts].TransformationDetails = &CurrentGameState->UnitTransformationDetails;
		CurrentGameState->CurrentScriptPool.Scripts[CurrentGameState->CurrentScriptPool.NumberOfScripts].StaticVariables = CurrentGameState->CurrentScriptPool.StaticVariables;
		CurrentGameState->CurrentScriptPool.Scripts[CurrentGameState->CurrentScriptPool.NumberOfScripts].NumberOfStaticVariables = CurrentGameState->CurrentUnitScript.NumberOfStatics;
		CurrentGameState->CurrentScriptPool.NumberOfScripts++;
	    }
	}
    }
    
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
    if(Input->KeyIsDown[SDLK_h] && !Input->KeyWasDown[SDLK_h])
    {
	Side --;
	if(Side < 0)
	    Side =9;
    }
    if(Input->KeyIsDown[SDLK_j] && !Input->KeyWasDown[SDLK_j])
    {
	Side ++;
	if(Side > 9)
	    Side =0;
    }
    //HACK: FIX THIS SHIT
     if(Input->KeyIsDown[SDLK_UP&255])
    {
	CurrentGameState->CameraXRotation += 0.01f;
    }
    if(Input->KeyIsDown[SDLK_DOWN&255])
    {
	CurrentGameState->CameraXRotation -= 0.01f;
    }
    if(Input->KeyIsDown[SDLK_LEFT&255])
    {
	CurrentGameState->CameraYRotation +=0.01f;
    }
    if(Input->KeyIsDown[SDLK_RIGHT&255])
    {
	CurrentGameState->CameraYRotation -=0.01f;
    }
    if(Input->KeyIsDown[SDLK_w])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]-=DZ[i];
    }
    if(Input->KeyIsDown[SDLK_s])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]+=DZ[i];
    }
    if(Input->KeyIsDown[SDLK_a])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]-=DX[i];
    }
    if(Input->KeyIsDown[SDLK_d])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]+=DX[i];
    }
    if(Input->KeyIsDown[SDLK_q])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]+=DY[i];
    }
    if(Input->KeyIsDown[SDLK_e])
    {
	for(int i=0;i<3;i++)
	    CurrentGameState->CameraTranslation[i]-=DY[i];
    }

}

void SetupDebugAxisBuffer(GLuint * DebugAxisBuffer)
{
        //Setup Debug axis buffer details:
    GLfloat LineData[3*2 * (3+2+3)];
    int CurrentLine =0;
    const int SCALE = 3;
    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1*SCALE;
    LineData[CurrentLine*2*8 + 4]= -1*SCALE;
	    
    LineData[CurrentLine*2*8 + 5]= 1*SCALE;
    LineData[CurrentLine*2*8 + 6]= 0;
    LineData[CurrentLine*2*8 + 7]= 0;

	    
    LineData[CurrentLine*2*8 + 8] = 1*SCALE;
    LineData[CurrentLine*2*8 + 9]= 0;
    LineData[CurrentLine*2*8 + 10]= 0;

    LineData[CurrentLine*2*8 + 11]= -1*SCALE;
    LineData[CurrentLine*2*8 + 12]= -1*SCALE;

    LineData[CurrentLine*2*8 + 13]= 1*SCALE;
    LineData[CurrentLine*2*8 + 14]= 0;
    LineData[CurrentLine*2*8 + 15]= 0;
    CurrentLine++;

    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1*SCALE;
    LineData[CurrentLine*2*8 + 4]= -1*SCALE;
	    
    LineData[CurrentLine*2*8 + 5]= 0;
    LineData[CurrentLine*2*8 + 6]= 1*SCALE;
    LineData[CurrentLine*2*8 + 7]= 0;

	    
    LineData[CurrentLine*2*8 + 8] = 0;
    LineData[CurrentLine*2*8 + 9]= 1*SCALE;
    LineData[CurrentLine*2*8 + 10]= 0;

    LineData[CurrentLine*2*8 + 11]= -1*SCALE;
    LineData[CurrentLine*2*8 + 12]= -1*SCALE;

    LineData[CurrentLine*2*8 + 13]= 0;
    LineData[CurrentLine*2*8 + 14]= 1*SCALE;
    LineData[CurrentLine*2*8 + 15]= 0;
    CurrentLine++;

    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1*SCALE;
    LineData[CurrentLine*2*8 + 4]= -1*SCALE;
	    
    LineData[CurrentLine*2*8 + 5]= 0;
    LineData[CurrentLine*2*8 + 6]= 0;
    LineData[CurrentLine*2*8 + 7]= 1*SCALE;

	    
    LineData[CurrentLine*2*8 + 8] = 0;
    LineData[CurrentLine*2*8 + 9]= 0;
    LineData[CurrentLine*2*8 + 10]= 1*SCALE;

    LineData[CurrentLine*2*8 + 11]= -1*SCALE;
    LineData[CurrentLine*2*8 + 12]= -1*SCALE;

    LineData[CurrentLine*2*8 + 13]= 0;
    LineData[CurrentLine*2*8 + 14]= 0;
    LineData[CurrentLine*2*8 + 15]= 1*SCALE;
    CurrentLine++;

    GLuint VertexBuffer;
    glGenVertexArrays(1,DebugAxisBuffer);
    glBindVertexArray(*DebugAxisBuffer);

    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*3*(3+2+3)*2,LineData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),(GLvoid*)(sizeof(GLfloat)*5));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glDeleteBuffers(1,&VertexBuffer);
}

void SetupGameState( GameState * CurrentGameState)
{
    CurrentGameState->ProjectionMatrix.SetProjection(60,float(CurrentGameState->ScreenWidth)/CurrentGameState->ScreenHeight,1.0,10000.0);

//    CurrentGameState->ViewMatrix->SetTranslation(1,-2.5,2);
    //  CurrentGameState->ViewMatrix->Rotate(0,1,0, (float)PI*0.75f);
    CurrentGameState->CameraYRotation = 0;//1.25*PI;
//	CurrentGameState->CameraXRotation += 0.1f;

    CurrentGameState->CameraTranslation[0] =4.0f;
    CurrentGameState->CameraTranslation[1] =40.0f;
    CurrentGameState->CameraTranslation[2] =50.0f;

    CurrentGameState->UnitIndex=14;
    
    //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SetupDebugAxisBuffer(&CurrentGameState->DebugAxisBuffer);

}

void InitialiseGame(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    CurrentGameState->IsInitialised=1;
    InitializeArena(&CurrentGameState->GameArena,GameMemory->PermanentStoreSize-sizeof(GameState),GameMemory->PermanentStore+sizeof(GameState));
    InitializeArena(&CurrentGameState->TempArena,GameMemory->TransientStoreSize-sizeof(GameState),GameMemory->TransientStore);
    SetupGameState(CurrentGameState);
}

float dot3(float * v1, float * v2)
{
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

Matrix FPSViewMatrix(float * eye, float pitch, float yaw)
{
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
 
    float xaxis[3] = { cosYaw, 0, -sinYaw };
    float yaxis[3] = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    float zaxis[3] = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
 
    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    Matrix viewMatrix;
    float MatDeets[]={
             xaxis[0],            yaxis[0],            zaxis[0],      0 ,
               xaxis[1],            yaxis[1],            zaxis[1],      0 ,
               xaxis[2],            yaxis[2],            zaxis[2],      0 ,
         -dot3( xaxis, eye ), -dot3( yaxis, eye ), -dot3( zaxis, eye ), 1 
    };

    for(int i=0;i<4;i++)
    {
	for(int j=0;j<4;j++)
	    viewMatrix.Contents[i*4+j]=MatDeets[j*4+i];
    }

     
    return viewMatrix;
}

extern "C"
{
    void GameUpdateAndRender(InputState * Input, Memory * GameMemory)
    {
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	if(!CurrentGameState->IsInitialised)
	{
	    InitialiseGame(GameMemory);
	}
    
	HandleInput(Input,CurrentGameState);
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(CurrentGameState->UnitShaderDetails.Shader->ProgramID);
	glBindTexture(GL_TEXTURE_2D,CurrentGameState->UnitTextures.Texture);
	CurrentGameState->ProjectionMatrix.Upload(CurrentGameState->UnitShaderDetails.ProjectionMatrixLocation);


	//CurrentGameState->ViewMatrix->Rotate(0,1,0, PI/300);
	//CurrentGameState->ViewMatrix->Move(0.01,-0.01,-0.01);
	CurrentGameState->ViewMatrix = FPSViewMatrix(CurrentGameState->CameraTranslation, CurrentGameState->CameraXRotation, CurrentGameState->CameraYRotation);
	CurrentGameState->ViewMatrix.Upload(CurrentGameState->UnitShaderDetails.ViewMatrixLocation);
	
	Matrix ModelMatrix;

	CleanUpScriptPool(&CurrentGameState->CurrentScriptPool);
	
	for(int i=0;i<CurrentGameState->CurrentScriptPool.NumberOfScripts;i++)
	{
	    RunScript(&CurrentGameState->CurrentUnitScript, &CurrentGameState->CurrentScriptPool.Scripts[i], &CurrentGameState->temp_model, &CurrentGameState->CurrentScriptPool);
	}
	b32 Animate = CurrentGameState->NumberOfFrames%10==0;

	UpdateTransformationDetails(&CurrentGameState->temp_model,&CurrentGameState->UnitTransformationDetails,1.0f/60.0f, Animate);
	for(int x=0;x<5;x++)
	{
	    for(int y=0;y<10;y++)
	    {
		ModelMatrix.SetTranslation(30.5f+x*50,14.5f,23.4f+y*40);
		RenderObject3d(&CurrentGameState->temp_model,&CurrentGameState->UnitTransformationDetails,CurrentGameState->UnitShaderDetails.ModelMatrixLocation,CurrentGameState->PaletteData,CurrentGameState->DebugAxisBuffer,Side, &CurrentGameState->UnitTextures,&CurrentGameState->TempArena,Matrix(),Matrix(),ModelMatrix);
	    }
	}
	
	glUseProgram(CurrentGameState->MapShader->ProgramID);
	CurrentGameState->ProjectionMatrix.Upload(GetUniformLocation(CurrentGameState->MapShader,"Projection"));
	CurrentGameState->ViewMatrix.Upload(GetUniformLocation(CurrentGameState->MapShader,"View"));
	CurrentGameState->TestMap.Render(CurrentGameState->MapShader);
    

	//TODO(Christof): Unit Rendering here



	//NOTE(Christof): All 2D Rendering to be done after this, all 3D before
	//TODO(Christof): Make the division here clearer?
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);                      // Turn Blending on
	glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off


	for(int i=0;i<CurrentGameState->CurrentUnitScript.NumberOfFunctions;i++)
	{
	    //TODO(Christof): Display Scripts as before?
	    const int MAX_SCRIPT_STRING_LENGTH = 128;
	    char ScriptString[MAX_SCRIPT_STRING_LENGTH];
	    snprintf(ScriptString,MAX_SCRIPT_STRING_LENGTH, "%d) %s", i, CurrentGameState->CurrentUnitScript.FunctionNames[i] );
	    Color TextColor = {{1,1,1}};
	    
	    Block BlockedOn = BLOCK_INIT;
	    for(int j=0;j<CurrentGameState->CurrentScriptPool.NumberOfScripts;j++)
	    {
		if(CurrentGameState->CurrentScriptPool.Scripts[j].ScriptNumber == i )
		    BlockedOn=CurrentGameState->CurrentScriptPool.Scripts[j].BlockedOn;
	    }
	    switch(BlockedOn)
	    {
	    case BLOCK_INIT:
		TextColor = {{1,1,1}};
		break;
		//GREEN
	    case BLOCK_NOT_BLOCKED:
		TextColor = {{0,1,0}};
		break;
		//CYAN
	    case BLOCK_MOVE:
		TextColor = {{0,1,1}};
		break;
		//PURPLE
	    case BLOCK_TURN:
		TextColor = {{1,0,1}};
		break;
//BLUE
	    case BLOCK_SLEEP:
		TextColor = {{0,0,1}};
		break;
//YELLOW
	    case BLOCK_DONE:
		TextColor = {{1,1,0}};
		break;
		//RED
	    case BLOCK_SCRIPT:
		TextColor = {{1,0,0}};
		break;
	    }
	    DrawTextureFontText(ScriptString, CurrentGameState->ScreenWidth - 150,150+i*30,&CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails , 1.0f,  TextColor );
	}
	//TODO(Christof): free memory correctly
	int Index=CurrentGameState->UnitIndex;
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
	char * Desc=CurrentGameState->Units.Details[Index].GetString("Description");
	int size=snprintf(NULL, 0, "%s: %s\n%s",SideName,Name,Desc)+1;
	//char tmp[size];
	char * tmp = PushArray(&CurrentGameState->TempArena, size, char);
	float Alpha = 1.0f ;

	snprintf(tmp,size,"%s: %s\n%s",SideName, Name,Desc);
	DrawTextureFontText(tmp, 15, 54, &CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails, Alpha);
	PopArray(&CurrentGameState->TempArena, tmp, size, char);


	static int GUIIndex = 0;
	if(CurrentGameState->NumberOfFrames%30 ==0)
	    GUIIndex++;
	if(GUIIndex > CurrentGameState->NumberOfGuis)
	    GUIIndex =0;
		
	//RenderTAUIElement(&CurrentGameState->GUIs[GUIIndex],&CurrentGameState->DrawTextureShaderDetails, &CurrentGameState->Font12, &CurrentGameState->CommonGUITextures);

	static u64 CurrentFrameTime = 0;
	static u64 LastFrameTime =0;
	static float CurrentFPS = 0;

	CurrentFrameTime = GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);

	char FPS[32];
	const float FramesToCount = 30.0f;
	CurrentFPS = (CurrentFPS*(FramesToCount-1) + 1.0f/((CurrentFrameTime - LastFrameTime)/1000.0f))/FramesToCount;
	snprintf(FPS, 32, "%0.2f, %ld", CurrentFPS, CurrentFrameTime - LastFrameTime);
	DrawTextureFontText(FPS, 0,0,&CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails, 1.0f);
	LastFrameTime = CurrentFrameTime;

	char MemoryUsageText[128];

	snprintf(MemoryUsageText, 128, "Game Arena: %.2fMB of %.2fMB (%.2f%% free)\nTemp Arena: %.2fMB of %.2fMB (%.2f%% free)\nScript Pool Size: %d",
		 CurrentGameState->GameArena.Used/(1024.0f*1024), CurrentGameState->GameArena.Size/(1024.0f*1024),
		 float(CurrentGameState->GameArena.Size - CurrentGameState->GameArena.Used)/CurrentGameState->GameArena.Size*100.0f,
		 CurrentGameState->TempArena.Used/(1024.0f*1024), CurrentGameState->TempArena.Size/(1024.0f*1024),
		 float(CurrentGameState->TempArena.Size - CurrentGameState->TempArena.Used)/CurrentGameState->TempArena.Size*100.0f,
	    CurrentGameState->CurrentScriptPool.NumberOfScripts);
	DrawTextureFontText(MemoryUsageText, CurrentGameState->ScreenWidth-TextSizeInPixels(MemoryUsageText, &CurrentGameState->Font12).Width, 0,&CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails, 1.0f);


	CurrentGameState->NumberOfFrames++;
    }
}
