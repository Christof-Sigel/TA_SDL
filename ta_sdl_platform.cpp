#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "Logging.h"
#include <SDL2/SDL_loadso.h>

#include "ta_sdl_platform.h"
#include "ta_sdl_game.h"


#include "sdl.cpp"

//TODO(Christof): Make these load from dynamic library

void (*GameSetup)(Memory * GameMemory) = NULL;
void (*CheckResources)(Memory * GameMemory) = NULL;
void (*GameUpdateAndRender)(InputState * Input, Memory* GameMemory) = NULL;
void (*GameTeardown)(Memory * GameMemory) = NULL;
void * LibObject=NULL;

void LoadGameLibrary()
{
    LibObject= SDL_LoadObject("./ta_sdl_game.so");
    if(!LibObject)
    {
	LogError("Failed to load DLL: %s",SDL_GetError());
	return;
    }
    GameSetup = (void(*)(Memory *))SDL_LoadFunction(LibObject,"GameSetup");
    if(!GameSetup)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    CheckResources = (void(*)(Memory *))SDL_LoadFunction(LibObject,"CheckResources");
    if(!CheckResources)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    GameUpdateAndRender = (void(*)(InputState*, Memory*))SDL_LoadFunction(LibObject,"GameUpdateAndRender");
    if(!GameUpdateAndRender)
    {
	LogError("Failed to load function pointer: %s",SDL_GetError());
	return;
    }
    GameTeardown = (void(*)(Memory *))SDL_LoadFunction(LibObject,"GameTeardown");
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
    SDL_UnloadObject(LibObject);
}

int main(int argc, char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	LogError("SDL_Init Error: %s", SDL_GetError());
	return 0;
    }
    LoadGameLibrary();
 
    InputState GameInputState={};
    Memory GameMemory={};

    GameMemory.PermanentStoreSize = 256 * 1024 * 1024;//TODO(Christof): convenience macros for size
    GameMemory.TransientStoreSize = 64 * 1024 * 1024;

    uint64_t TotalSize = GameMemory.PermanentStoreSize + GameMemory.TransientStoreSize;
    GameMemory.PermanentStore = (uint8_t*)calloc(1,TotalSize);
    GameMemory.TransientStore = GameMemory.PermanentStore+GameMemory.PermanentStoreSize;

    GameState * CurrentGameState=(GameState *)GameMemory.PermanentStore;
    
    if(!SetupSDLWindow(CurrentGameState))
	return 1;
    
    GameSetup(&GameMemory);
    SDL_Event e;
    while( !CurrentGameState->Quit )
    {
	CheckResources(&GameMemory);
	UnloadGameLibrary();
	LoadGameLibrary();

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
	SDL_GL_SwapWindow( MainSDLWindow );
	for(int i=0;i<256;i++)
	{
	    GameInputState.KeyWasDown[i]=GameInputState.KeyIsDown[i];
	}
    }
    GameTeardown(&GameMemory);
    SDL_DestroyWindow(MainSDLWindow);
    SDL_Quit();

    return 0;
}
