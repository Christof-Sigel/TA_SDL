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
	    LoadUnitScriptFromBuffer(CurrentGameState->CurrentUnitScript, ScriptBuffer,&CurrentGameState->GameArena);
	    memset(CurrentGameState->CurrentScriptPool,0,sizeof(ScriptStatePool));
	    
	    for(int i=0;i<CurrentGameState->CurrentUnitScript->NumberOfFunctions;i++)
	    {
		int size=snprintf(NULL, 0, "%d) %s",i,CurrentGameState->CurrentUnitScript->FunctionNames[i])+2;
		//char tmp[size];
		STACK_ARRAY(tmp,size,char);
		snprintf(tmp,size,"%d) %s",i,CurrentGameState->CurrentUnitScript->FunctionNames[i]);
		CurrentGameState->UnitDetailsText[i]=SetupOnScreenText(tmp,float(CurrentGameState->ScreenWidth-220), float(i*CurrentGameState->Times24->Height*2+40), 1,1,1, CurrentGameState->Times24);
	    }
	    if(CurrentGameState->temp_model->Vertices)
		Unload3DO(CurrentGameState->temp_model);
	    Load3DOFromBuffer(temp,CurrentGameState->temp_model,CurrentGameState->NextTexture,CurrentGameState->Textures,&CurrentGameState->GameArena);
	    InitTransformationDetails(CurrentGameState->temp_model, CurrentGameState->UnitTransformationDetails, &CurrentGameState->GameArena);
	    int ScriptNum = GetScriptNumberForFunction(CurrentGameState->CurrentUnitScript,"Create");
	    if(ScriptNum !=-1)
	    {
		CurrentGameState->CurrentScriptPool->NumberOfScripts = 1;
		CurrentGameState->CurrentScriptPool->Scripts[0].ScriptNumber = ScriptNum;
		CurrentGameState->CurrentScriptPool->Scripts[0].TransformationDetails = CurrentGameState->UnitTransformationDetails;
		CurrentGameState->CurrentScriptPool->Scripts[0].StaticVariables = PushArray(&CurrentGameState->GameArena, CurrentGameState->CurrentUnitScript->NumberOfStatics, int32_t);
		CurrentGameState->CurrentScriptPool->Scripts[0].NumberOfStaticVariables = CurrentGameState->CurrentUnitScript->NumberOfStatics;
	    }


	    

	

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

		int size=snprintf(NULL, 0, "%s: %s",SideName,Name, Index, CurrentGameState->Units.Size)+1;
		//char tmp[size];
		STACK_ARRAY(tmp,size,char);
		snprintf(tmp,size,"%s: %s",SideName,Name, Index, CurrentGameState->Units.Size);
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
//	    PrepareObject3dForRendering(CurrentGameState->temp_model,CurrentGameState->PaletteData);
	}
    }
}

static int32_t Side=0;

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
	    if(ScriptNumber < CurrentGameState->CurrentUnitScript->NumberOfFunctions)
	    {

		CurrentGameState->CurrentScriptPool->Scripts[CurrentGameState->CurrentScriptPool->NumberOfScripts].ScriptNumber = ScriptNumber;
		CurrentGameState->CurrentScriptPool->Scripts[CurrentGameState->CurrentScriptPool->NumberOfScripts].TransformationDetails = CurrentGameState->UnitTransformationDetails;
		CurrentGameState->CurrentScriptPool->Scripts[CurrentGameState->CurrentScriptPool->NumberOfScripts].StaticVariables = PushArray(&CurrentGameState->GameArena, CurrentGameState->CurrentUnitScript->NumberOfStatics, int32_t);
		CurrentGameState->CurrentScriptPool->Scripts[CurrentGameState->CurrentScriptPool->NumberOfScripts].NumberOfStaticVariables = CurrentGameState->CurrentUnitScript->NumberOfStatics;
		CurrentGameState->CurrentScriptPool->NumberOfScripts++;
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
    int32_t CurrentLine =0;
    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1;
    LineData[CurrentLine*2*8 + 4]= -1;
	    
    LineData[CurrentLine*2*8 + 5]= 1;
    LineData[CurrentLine*2*8 + 6]= 0;
    LineData[CurrentLine*2*8 + 7]= 0;

	    
    LineData[CurrentLine*2*8 + 8] = 1;
    LineData[CurrentLine*2*8 + 9]= 0;
    LineData[CurrentLine*2*8 + 10]= 0;

    LineData[CurrentLine*2*8 + 11]= -1;
    LineData[CurrentLine*2*8 + 12]= -1;

    LineData[CurrentLine*2*8 + 13]= 1;
    LineData[CurrentLine*2*8 + 14]= 0;
    LineData[CurrentLine*2*8 + 15]= 0;
    CurrentLine++;

    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1;
    LineData[CurrentLine*2*8 + 4]= -1;
	    
    LineData[CurrentLine*2*8 + 5]= 0;
    LineData[CurrentLine*2*8 + 6]= 1;
    LineData[CurrentLine*2*8 + 7]= 0;

	    
    LineData[CurrentLine*2*8 + 8] = 0;
    LineData[CurrentLine*2*8 + 9]= 1;
    LineData[CurrentLine*2*8 + 10]= 0;

    LineData[CurrentLine*2*8 + 11]= -1;
    LineData[CurrentLine*2*8 + 12]= -1;

    LineData[CurrentLine*2*8 + 13]= 0;
    LineData[CurrentLine*2*8 + 14]= 1;
    LineData[CurrentLine*2*8 + 15]= 0;
    CurrentLine++;

    LineData[CurrentLine*2*8 + 0] = 0;
    LineData[CurrentLine*2*8 + 1]= 0;
    LineData[CurrentLine*2*8 + 2]= 0;

    LineData[CurrentLine*2*8 + 3]= -1;
    LineData[CurrentLine*2*8 + 4]= -1;
	    
    LineData[CurrentLine*2*8 + 5]= 0;
    LineData[CurrentLine*2*8 + 6]= 0;
    LineData[CurrentLine*2*8 + 7]= 1;

	    
    LineData[CurrentLine*2*8 + 8] = 0;
    LineData[CurrentLine*2*8 + 9]= 0;
    LineData[CurrentLine*2*8 + 10]= 1;

    LineData[CurrentLine*2*8 + 11]= -1;
    LineData[CurrentLine*2*8 + 12]= -1;

    LineData[CurrentLine*2*8 + 13]= 0;
    LineData[CurrentLine*2*8 + 14]= 0;
    LineData[CurrentLine*2*8 + 15]= 1;
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


    
//    CurrentGameState->ViewMatrix->SetTranslation(1,-2.5,2);
    //  CurrentGameState->ViewMatrix->Rotate(0,1,0, (float)PI*0.75f);
    CurrentGameState->CameraYRotation = 0;//1.25*PI;
//	CurrentGameState->CameraXRotation += 0.1f;

    CurrentGameState->CameraTranslation[0] =4.0f;
    CurrentGameState->CameraTranslation[1] =40.0f;
    CurrentGameState->CameraTranslation[2] =50.0f;


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
    CurrentGameState->UnitTransformationDetails = PushStruct(GameArena, Object3dTransformationDetails);
    CurrentGameState->CurrentScriptPool = PushStruct(GameArena, ScriptStatePool);
    CurrentGameState->ScriptBackground = PushStruct(GameArena, UIElement);
    *CurrentGameState->ScriptBackground = SetupUIElement(float(CurrentGameState->ScreenWidth -240),0, 240, float(CurrentGameState->ScreenHeight), 0,0,0, 1,1,1, 1.0, 1.0);

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
	glUseProgram(CurrentGameState->UnitShader->ProgramID);
	glBindTexture(GL_TEXTURE_2D,CurrentGameState->UnitTexture);
	CurrentGameState->ProjectionMatrix->Upload(CurrentGameState->ProjectionMatrixLocation);


	//CurrentGameState->ViewMatrix->Rotate(0,1,0, PI/300);
	//CurrentGameState->ViewMatrix->Move(0.01,-0.01,-0.01);
	*CurrentGameState->ViewMatrix = FPSViewMatrix(CurrentGameState->CameraTranslation, CurrentGameState->CameraXRotation, CurrentGameState->CameraYRotation);
	CurrentGameState->ViewMatrix->Upload(CurrentGameState->ViewMatrixLocation);
	
	Matrix ModelMatrix;
	ModelMatrix.SetTranslation(30.5,14.5,23.4);

	for(int i=0;i<CurrentGameState->CurrentScriptPool->NumberOfScripts;i++)
	{
	    RunScript(CurrentGameState->CurrentUnitScript, &CurrentGameState->CurrentScriptPool->Scripts[i], CurrentGameState->temp_model, CurrentGameState->CurrentScriptPool);
	    ScreenText * ScriptText = &CurrentGameState->UnitDetailsText[CurrentGameState->CurrentScriptPool->Scripts[i].ScriptNumber];
	    switch(CurrentGameState->CurrentScriptPool->Scripts[i].BlockedOn)
	    {
		//Green - this will at the moment never happen
	    case BLOCK_NOT_BLOCKED:
		ScriptText->Color = {{0,1,0}};
		break;
		//CYAN
	    case BLOCK_MOVE:
		ScriptText->Color = {{0,1,1}};
		break;
		//PURPLE
	    case BLOCK_TURN:
		ScriptText->Color = {{1,0,1}};
		break;
//BLUE
	    case BLOCK_SLEEP:
		ScriptText->Color = {{0,0,1}};
		break;
//YELLOW
	    case BLOCK_DONE:
		ScriptText->Color = {{1,1,0}};
		break;
		//RED
	    case BLOCK_SCRIPT:
		ScriptText->Color = {{1,0,0}};
		break;



	    }
	}
	int32_t Animate = CurrentGameState->NumberOfFrames%10==0;
//	int32_t Animate =0;


	UpdateTransformationDetails(CurrentGameState->temp_model,CurrentGameState->UnitTransformationDetails,1.0f/60.0f);
	#if TEXTURE_DEBUG
	if(!DEBUG_done)
	{
	    LogDebug("\n\n\n\n\n");
	}
	#endif
	RenderObject3d(CurrentGameState->temp_model,CurrentGameState->UnitTransformationDetails,CurrentGameState->ModelMatrixLocation,CurrentGameState->PaletteData,CurrentGameState->DebugAxisBuffer,Animate,Side,ModelMatrix);
	#if TEXTURE_DEBUG
	DEBUG_done = 1;
	#endif
	
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
	RenderUIElement(*CurrentGameState->ScriptBackground, CurrentGameState->UIElementShaderProgram,CurrentGameState->UIElementPositionLocation, CurrentGameState->UIElementSizeLocation,  CurrentGameState->UIElementColorLocation, CurrentGameState->UIElementBorderColorLocation, CurrentGameState->UIElementBorderWidthLocation,  CurrentGameState->UIElementAlphaLocation,  CurrentGameState->UIElementRenderingVertexBuffer, CurrentGameState->FontShader,  CurrentGameState->FontPositionLocation,  CurrentGameState->FontColorLocation);
	for(int i=0;i<CurrentGameState->CurrentUnitScript->NumberOfFunctions;i++)
	    RenderOnScreenText(CurrentGameState->UnitDetailsText[i], CurrentGameState->FontShader, CurrentGameState->FontPositionLocation, CurrentGameState->FontColorLocation);

	CurrentGameState->NumberOfFrames++;
    }

}
