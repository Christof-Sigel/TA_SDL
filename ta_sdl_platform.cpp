#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "Logging.h"

#include "ta_sdl_platform.h"
#include "ta_sdl_game.h"


#include "sdl.cpp"

//TODO(Christof): Make these load from dynamic library
void GameSetup(Memory * GameMemory);
void CheckResources(Memory * GameMemory);
void GameUpdateAndRender(InputState *Input, Memory * GameMemory);
void GameTeardown(Memory * GameMemory);



int main(int argc, char * argv[])
{



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
