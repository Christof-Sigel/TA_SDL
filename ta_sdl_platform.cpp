#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_loadso.h>

#include "ta_sdl_platform.h"
#include "ta_sdl_game.h"


#include "sdl.cpp"
#define QUOTE2(X) #X
#define QUOTE(X) QUOTE2(X)
#define GAME_LIBRARY_OBJECT QUOTE(DLL_NAME)
#define TEMP_GAME_LIBRARY_OBJECT QUOTE(DLL_TEMP_NAME)

void (*GameSetup)(Memory * GameMemory) = NULL;
void (*CheckResources)(Memory * GameMemory) = NULL;
void (*GameUpdateAndRender)(InputState * Input, Memory* GameMemory) = NULL;
void (*GameTeardown)(Memory * GameMemory) = NULL;
void * GameLibraryObject=NULL;
s64  GameLibraryObjectModifyTime=0;

#ifdef __WINDOWS__
#include <windows.h>
#endif

void LoadGameLibrary()
{

    GameLibraryObjectModifyTime = GetFileModifiedTime(GAME_LIBRARY_OBJECT);
#ifdef __WINDOWS__
    CopyFile(GAME_LIBRARY_OBJECT, TEMP_GAME_LIBRARY_OBJECT, FALSE);
    GameLibraryObject= SDL_LoadObject(TEMP_GAME_LIBRARY_OBJECT);
#else
    
    GameLibraryObject= SDL_LoadObject(GAME_LIBRARY_OBJECT);
#endif
    if(!GameLibraryObject)
    {
	LogError("Failed to load DLL: %s",SDL_GetError());
	return;
    }
    GameSetup = (void(*)(Memory *))SDL_LoadFunction(GameLibraryObject,"GameSetup");
    if(!GameSetup)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    CheckResources = (void(*)(Memory *))SDL_LoadFunction(GameLibraryObject,"CheckResources");
    if(!CheckResources)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    GameUpdateAndRender = (void(*)(InputState*, Memory*))SDL_LoadFunction(GameLibraryObject,"GameUpdateAndRender");
    if(!GameUpdateAndRender)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    GameTeardown = (void(*)(Memory *))SDL_LoadFunction(GameLibraryObject,"GameTeardown");
    if(!GameTeardown)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
}

void UnloadGameLibrary()
{
    GameSetup=0;
    CheckResources=0;
    GameUpdateAndRender=0;
    GameTeardown=0;
    SDL_UnloadObject(GameLibraryObject);
}

inline b32 HasGameLibraryBeenUpdated()
{
#ifdef __WINDOWS__
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(GetFileAttributesEx("lock.tmp", GetFileExInfoStandard, &Ignored))
	return 0;
#endif
    s64  CurrentModifyTime = GetFileModifiedTime(GAME_LIBRARY_OBJECT);
    return CurrentModifyTime > GameLibraryObjectModifyTime;
}


int main(int argc, char * argv[])
{
    int x = argc;
    char * temp = argv[x];
    temp++;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	LogError("SDL_Init Error: %s", SDL_GetError());
	return 0;
    }
    LoadGameLibrary();
 
    InputState GameInputState={};
    Memory GameMemory={};

    GameMemory.PermanentStoreSize = 256 * 1024 * 1024;//TODO(Christof): convenience macros for size
    GameMemory.TransientStoreSize = 256 * 1024 * 1024;

    u64  TotalSize = GameMemory.PermanentStoreSize + GameMemory.TransientStoreSize;
    GameMemory.PermanentStore = (u8 *)calloc(1,TotalSize);
    GameMemory.TransientStore = GameMemory.PermanentStore+GameMemory.PermanentStoreSize;

    GameState * CurrentGameState=(GameState *)GameMemory.PermanentStore;
    
    if(!SetupSDLWindow(CurrentGameState))
	return 1;
    
    GameSetup(&GameMemory);
    SDL_Event e;
    while( !CurrentGameState->Quit )
    {
	CheckResources(&GameMemory);
	if(HasGameLibraryBeenUpdated())
	{
	    UnloadGameLibrary();
	    LoadGameLibrary();
	}

	while( SDL_PollEvent( &e ) != 0 )
	{
	    if( e.type == SDL_QUIT )
	    {
		CurrentGameState->Quit=1;
	    }
	    else if( e.type == SDL_KEYDOWN)
	    {
		HandleKeyDown(e.key.keysym,&GameInputState);
	    }
	    else if( e.type == SDL_KEYUP)
	    {
		HandleKeyUp(e.key.keysym,&GameInputState);
	    }
	}
	

	GameUpdateAndRender(&GameInputState,&GameMemory);
	SDL_GL_SwapWindow( CurrentGameState->MainSDLWindow );
	for(int i=0;i<256;i++)
	{
	    GameInputState.KeyWasDown[i]=GameInputState.KeyIsDown[i];
	}
    }
    GameTeardown(&GameMemory);
    SDL_DestroyWindow(CurrentGameState->MainSDLWindow);
    SDL_Quit();

    return 0;
}

int wmain(int argc, char * argv[])
{
    return  main(argc, argv);
}
