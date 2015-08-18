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
#include "platform_code.cpp"

#include "GL.cpp"
#include "file_formats.cpp"


#include "ta_sdl_game_load.cpp"



internal Matrix FPSViewMatrix(float * eye, float pitch, float yaw);
internal void HandleInput(InputState * Input, GameState * CurrentGameState)
{
    b32 MouseButtonDown = Input->MouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT);
    b32 MouseButtonClicked = Input->LastMouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT) && !(Input->MouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT));
    switch(CurrentGameState->State)
    {
    case STATE_RUNNING:
    case STATE_PAUSED:
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


	if(Input->KeyIsDown[SDLK_p]&& !Input->KeyWasDown[SDLK_p])
	{
	    if(CurrentGameState->State == STATE_RUNNING)
	    {
		CurrentGameState->State = STATE_PAUSED;
	    }
	    else if(CurrentGameState->State == STATE_PAUSED)
	    {
		CurrentGameState->State = STATE_RUNNING;
	    }
	}

	if(Input->KeyIsDown[SDLK_ESCAPE] && !Input->KeyWasDown[SDLK_ESCAPE]  )
	{
	    if(CurrentGameState->State == STATE_RUNNING || CurrentGameState->State == STATE_PAUSED)
		CurrentGameState->State = STATE_MAIN_MENU;
	    else
		CurrentGameState->State = STATE_RUNNING;
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

    break;


    case STATE_MAIN_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->MainMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName)
	    {
	    case ELEMENT_NAME_EXIT:
		CurrentGameState->State = STATE_QUIT;
		break;
	    case ELEMENT_NAME_SINGLEPLAYER:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    case ELEMENT_NAME_MULTIPLAYER:
		//TODO
		break;
	    case ELEMENT_NAME_INTRO:
		//TODO
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;
    case STATE_SINGLEPLAYER_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->SinglePlayerMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName)
	    {
	    case ELEMENT_NAME_PREVIOUS_MENU:
		CurrentGameState->State = STATE_MAIN_MENU;
		break;
	    case ELEMENT_NAME_OPTIONS:
		CurrentGameState->State = STATE_OPTIONS_MENU;
		break;
	    case ELEMENT_NAME_NEW_CAMPAIGN:
		CurrentGameState->State = STATE_CAMPAIGN_MENU;
		break;
	    case ELEMENT_NAME_SKIRMISH:
		CurrentGameState->State = STATE_SKIRMISH_MENU;
		break;
	    case ELEMENT_NAME_LOAD_GAME:
		CurrentGameState->State = STATE_LOAD_GAME_MENU;
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;
    case STATE_OPTIONS_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->OptionsMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName)
	    {
	    case ELEMENT_NAME_PREVIOUS_MENU:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    case ELEMENT_NAME_CANCEL:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;

    case STATE_CAMPAIGN_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->CampaignMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName)
	    {
	    case ELEMENT_NAME_PREVIOUS_MENU:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    case ELEMENT_NAME_START:
		CurrentGameState->State = STATE_RUNNING;
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;

    case STATE_SKIRMISH_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->SkirmishMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked  && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName)
	    {
	    case ELEMENT_NAME_PREVIOUS_MENU:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;
    case STATE_LOAD_GAME_MENU:
    {
	TAUIElement * Element = ProcessMouse(&CurrentGameState->LoadGameMenu, Input->MouseX, Input->MouseY, MouseButtonDown,MouseButtonClicked, (CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2);
	if(MouseButtonClicked  && Element)
	{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	    switch(Element->ElementName )
	    {
	    case ELEMENT_NAME_LOAD:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    case ELEMENT_NAME_CANCEL:
		CurrentGameState->State = STATE_SINGLEPLAYER_MENU;
		break;
	    }
#pragma clang diagnostic pop
	}
    }
    break;
    case STATE_QUIT:
	//NOTE(Christof): nothing to do, simply to silence warning
	break;
    }


    CurrentGameState->MouseX = Input->MouseX;
    CurrentGameState->MouseY = Input->MouseY;

}


internal void SetupDebugRectBuffer(GLuint * DebugRectBuffer)
{
    GLfloat RenderData[6*(2+4)];//6 Vert (2 triangles) each 2 position coords and 4 distance to edge "coords"

    glGenVertexArrays(1,DebugRectBuffer);

    GLfloat Vertices[]={0,0, 1,0, 1,1, 0,1};
    GLfloat EdgeDistance[]={0,1,1,0, 0,0,1,1, 1,0,0,1, 1,1,0,0};

    int Indexes1[]={0,3,1};
    for(int i=0;i<3;i++)
    {
	RenderData[i*(2+4)+0]=Vertices[Indexes1[i]*2+0];
	RenderData[i*(2+4)+1]=Vertices[Indexes1[i]*2+1];

	RenderData[i*(2+4)+2]=EdgeDistance[Indexes1[i]*4+0];
	RenderData[i*(2+4)+3]=EdgeDistance[Indexes1[i]*4+1];
	RenderData[i*(2+4)+4]=EdgeDistance[Indexes1[i]*4+2];
	RenderData[i*(2+4)+5]=EdgeDistance[Indexes1[i]*4+3];
    }

    int Indexes2[]={1,3,2};

    for(int i=0;i<3;i++)
    {
	RenderData[6*3+i*(2+4)+0]=Vertices[Indexes2[i]*2+0];
	RenderData[6*3+i*(2+4)+1]=Vertices[Indexes2[i]*2+1];

	RenderData[6*3+i*(2+4)+2]=EdgeDistance[Indexes2[i]*4+0];
	RenderData[6*3+i*(2+4)+3]=EdgeDistance[Indexes2[i]*4+1];
	RenderData[6*3+i*(2+4)+4]=EdgeDistance[Indexes2[i]*4+2];
	RenderData[6*3+i*(2+4)+5]=EdgeDistance[Indexes2[i]*4+3];
    }

    glBindVertexArray(*DebugRectBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);


    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*6*(2+4),RenderData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,0);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,(GLvoid*)(sizeof(GLfloat)*2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glDeleteBuffers(1,&VertexBuffer);
}

internal void SetupDebugAxisBuffer(GLuint * DebugAxisBuffer)
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

internal void SetupGameState( GameState * CurrentGameState)
{
    CurrentGameState->ProjectionMatrix.SetProjection(60,float(CurrentGameState->ScreenWidth)/CurrentGameState->ScreenHeight,1.0,10000.0);

//    CurrentGameState->ViewMatrix->SetTranslation(1,-2.5,2);
    //  CurrentGameState->ViewMatrix->Rotate(0,1,0, (float)PI*0.75f);
    CurrentGameState->CameraYRotation = 0;//1.25*PI;
//	CurrentGameState->CameraXRotation += 0.1f;

    CurrentGameState->CameraTranslation[0] =4.0f;
    CurrentGameState->CameraTranslation[1] =40.0f;
    CurrentGameState->CameraTranslation[2] =50.0f;


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

internal void InitialiseGame(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    CurrentGameState->IsInitialised=1;
    InitializeArena(&CurrentGameState->GameArena,GameMemory->PermanentStoreSize-sizeof(GameState),GameMemory->PermanentStore+sizeof(GameState));
    InitializeArena(&CurrentGameState->TempArena,GameMemory->TransientStoreSize-sizeof(GameState),GameMemory->TransientStore);
    SetupGameState(CurrentGameState);
}

internal float dot3(float * v1, float * v2)
{
    return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

internal Matrix FPSViewMatrix(float * eye, float pitch, float yaw)
{
    float cosPitch = (float)cos(pitch);
    float sinPitch = (float)sin(pitch);
    float cosYaw = (float)cos(yaw);
    float sinYaw = (float)sin(yaw);

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
    void GameUpdateAndRender(InputState * Input, Memory * GameMemory);
    void GameUpdateAndRender(InputState * Input, Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	if(!CurrentGameState->IsInitialised)
	{
	    InitialiseGame(GameMemory);
	}

	HandleInput(Input,CurrentGameState);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	switch(CurrentGameState->State)
	{
	case STATE_RUNNING:
	    CurrentGameState->ViewMatrix = FPSViewMatrix(CurrentGameState->CameraTranslation, CurrentGameState->CameraXRotation, CurrentGameState->CameraYRotation);
	    //TODO(Christof): Update game state
	[[clang::fallthrough]];
	case STATE_PAUSED:
	{
	    glUseProgram(CurrentGameState->MapShader->ProgramID);
	    CurrentGameState->ProjectionMatrix.Upload(GetUniformLocation(CurrentGameState->MapShader,"Projection"));
	    CurrentGameState->ViewMatrix.Upload(GetUniformLocation(CurrentGameState->MapShader,"View"));
	    CurrentGameState->TestMap.Render(CurrentGameState->MapShader);

	}
	break;
	default:
	    break;
	}


	//NOTE(Christof): All 2D Rendering to be done after this, all 3D before
	//TODO(Christof): Make the division here clearer?
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);                      // Turn Blending on
	glDisable(GL_DEPTH_TEST);        //Turn Depth Testing off

#define DEBUG_DRAW_STATS 1
	switch(CurrentGameState->State)
	{
	case STATE_PAUSED:
	case STATE_RUNNING:
	break;
	case STATE_MAIN_MENU:
	    RenderTAUIElement(&CurrentGameState->MainMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails, &CurrentGameState->Font11, &CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;

	case STATE_SINGLEPLAYER_MENU:
	    RenderTAUIElement(&CurrentGameState->SinglePlayerMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;
	case STATE_CAMPAIGN_MENU:
	    RenderTAUIElement(&CurrentGameState->CampaignMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;
	case STATE_LOAD_GAME_MENU:
	    RenderTAUIElement(&CurrentGameState->SinglePlayerMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);


	    RenderTAUIElement(&CurrentGameState->LoadGameMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;
	case STATE_SKIRMISH_MENU:
	    RenderTAUIElement(&CurrentGameState->SkirmishMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;
	case STATE_OPTIONS_MENU:
	    RenderTAUIElement(&CurrentGameState->OptionsMenu,(CurrentGameState->ScreenWidth - CurrentGameState->MainMenu.Width)/2,(CurrentGameState->ScreenHeight - CurrentGameState->MainMenu.Height)/2,&CurrentGameState->DrawTextureShaderDetails,  &CurrentGameState->Font11,&CurrentGameState->Font12, &CurrentGameState->CommonGUITextures, &CurrentGameState->DebugRectDetails);
	    break;

	case STATE_QUIT:
	    //NOTE(Christof): nothing to do, simply to silence warning
	    break;
	}



	//Debug Stats - FPS/Memory usage at present
#if DEBUG_DRAW_STATS
	static u64 CurrentFrameTime = 0;
	static u64 LastFrameTime =0;
	static float CurrentFPS = 0;

	CurrentFrameTime = GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);

	char FPS[32];
	const float FramesToCount = 30.0f;
	CurrentFPS = (CurrentFPS*(FramesToCount-1) + 1.0f/((CurrentFrameTime - LastFrameTime)/1000.0f))/FramesToCount;
	snprintf(FPS, 32, "%0.2f, %lu", CurrentFPS, CurrentFrameTime - LastFrameTime);
	DrawTextureFontText(FPS, 0,0,&CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails, 1.0f);
	LastFrameTime = CurrentFrameTime;

	char MemoryUsageText[128];

	snprintf(MemoryUsageText, 128, "Game Arena: %.2fMB of %.2fMB (%.2f%% free)\nTemp Arena: %.2fMB of %.2fMB (%.2f%% free)",
		 CurrentGameState->GameArena.Used/(1024.0f*1024), CurrentGameState->GameArena.Size/(1024.0f*1024),
		 float(CurrentGameState->GameArena.Size - CurrentGameState->GameArena.Used)/CurrentGameState->GameArena.Size*100.0f,
		 CurrentGameState->TempArena.Used/(1024.0f*1024), CurrentGameState->TempArena.Size/(1024.0f*1024),
		 float(CurrentGameState->TempArena.Size - CurrentGameState->TempArena.Used)/CurrentGameState->TempArena.Size*100.0f);
	DrawTextureFontText(MemoryUsageText, CurrentGameState->ScreenWidth-TextSizeInPixels(MemoryUsageText, &CurrentGameState->Font12).Width, 0,&CurrentGameState->Font12,&CurrentGameState->DrawTextureShaderDetails, 1.0f);
#endif

	CurrentGameState->NumberOfFrames++;
    }


}
