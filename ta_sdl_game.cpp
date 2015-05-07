#include <stdio.h>

#include "platform_code.cpp"
#include "GL.cpp"
#include "UI.cpp"
#include "file_formats.cpp"
#include "ta_sdl_game.h"
#include "ta_sdl_game_load.cpp"

void HandleKeyDown(SDL_Keysym key);
extern bool32 quit;
bool32 SetupSDLWindow();
void Render();
void Setup();
void Teardown();

void CheckResources();
void ReloadShaders();






Matrix ProjectionMatrix;
Matrix ModelMatrix;
Matrix ViewMatrix;



int UnitIndex=0;


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






const float DR=0.01,DG=0.02,DB=0.015;
float dr=DR,dg=DG,db=DB;

void HandleInput(InputState * Input)
{
    if(Input->KeyPressed[SDLK_ESCAPE])
    {
	quit=true;
    }
    if(Input->KeyPressed[SDLK_o])
    {
	UnitIndex--;;
	LoadCurrentModel();
    }
    if(Input->KeyPressed[SDLK_l])
    {
	UnitIndex++;
	LoadCurrentModel();
    }
    //HACK: FIX THIS SHIT
    if(Input->KeyPressed[SDLK_UP&255])
    {
	ViewMatrix.Rotate(1,0,0,-0.01f);
    }
    if(Input->KeyPressed[SDLK_DOWN&255])
    {
	ViewMatrix.Rotate(1,0,0,0.01f);
    }
    if(Input->KeyPressed[SDLK_LEFT&255])
    {
	ViewMatrix.Rotate(0,1,0,-0.01f);
    }
    if(Input->KeyPressed[SDLK_RIGHT&255])
    {
	ViewMatrix.Rotate(0,1,0,0.01f);
    }
    if(Input->KeyPressed[SDLK_w])
    {
	ViewMatrix.Move(0,0,0.1);
    }
    if(Input->KeyPressed[SDLK_s])
    {
	ViewMatrix.Move(0,0,-0.1);
    }
    if(Input->KeyPressed[SDLK_a])
    {
	ViewMatrix.Move(-0.1,0,0);
    }
    if(Input->KeyPressed[SDLK_d])
    {
	ViewMatrix.Move(0.1,0,0);
    }
    if(Input->KeyPressed[SDLK_q])
    {
	ViewMatrix.Move(0,-0.1,0);
    }
    if(Input->KeyPressed[SDLK_e])
    {
	ViewMatrix.Move(0,0.1,0);
    }

}


void SetupGameState()
{
    ProjectionMatrix.SetProjection(60,float(ScreenWidth)/ScreenHeight,1.0,1000.0);

    ViewMatrix.SetTranslation(0,0,-2);
    ViewMatrix.Rotate(1,0,0, 0.5);

        //GL Setup:
    glClearColor( 0.f, 0.f,0.f, 0.f );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

}

bool32 Initialized = 0;

void GameUpdateAndRender(InputState * Input)
{
    HandleInput(Input);
    if(!Initialized)
    {
	Initialized = 1;
	SetupGameState();
    }
    
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

