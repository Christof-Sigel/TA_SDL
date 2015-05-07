#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "Logging.h"

#include "ta_sdl_platform.h"
#include "ta_sdl_game.h"

InputState GlobalInputState;
#include "sdl.cpp"

//TODO(Christof): Make these load from dynamic library
void Setup();
void CheckResources();
void GameUpdateAndRender(InputState *Input);
void Teardown();



bool32 quit=0;


int main(int argc, char * argv[])
{
    if(!SetupSDLWindow())
	return 1;

    Setup();
    
    SDL_Event e;
    while( !quit )
    {
	CheckResources();
	while( SDL_PollEvent( &e ) != 0 )
	{
	    if( e.type == SDL_QUIT )
	    {
		quit = true;
	    }
	    else if( e.type == SDL_KEYDOWN)
	    {
		HandleKeyDown(e.key.keysym);
	    }
	    else if( e.type == SDL_KEYUP)
	    {
		HandleKeyUp(e.key.keysym);
	    }
	}
	GameUpdateAndRender(&GlobalInputState);
	SDL_GL_SwapWindow( MainSDLWindow );
    }

    Teardown();
    SDL_DestroyWindow(MainSDLWindow);
    SDL_Quit();

    return 0;
}
