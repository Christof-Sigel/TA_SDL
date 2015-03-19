#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>

void HandleKeyDown(SDL_Keysym key);
typedef int bool32;
bool32 quit=0;

int ScreenWidth=800,ScreenHeight=600;

int main(int argc, char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	printf("SDL_Init Error: %s\n", SDL_GetError());
	return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_Window * win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    if (!win)
    {
	printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
	return 1;
    }


    GLint GLMajorVer, GLMinorVer;
    
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	printf("failed before create context : %s\n", gluErrorString(ErrorValue));
    }
    

    SDL_GLContext gContext = SDL_GL_CreateContext( win );
    SDL_GL_MakeCurrent (win,gContext);
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
	printf("Warning: Unable to set VSync! SDL Error: %s\n",SDL_GetError());
    }

    glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVer);
    glGetIntegerv(GL_MINOR_VERSION, &GLMinorVer);
    printf("OpenGL Version %d.%d\n",GLMajorVer,GLMinorVer);


    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	printf("failed before glewInit : %s\n",gluErrorString(ErrorValue));
    }
  

    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
	printf("Error initializing GLEW! %s\n", glewGetErrorString( glewError ));
	return 1;
    }

    
    //as we need to use glewExperimental - known issue (it segfaults otherwise!) - we encounter
    //another known issue, which is that while glewInit suceeds, it leaves opengl in an error state
    ErrorValue = glGetError();
    
    
    //setup();


    
    

    //Event handler
    SDL_Event e;
		
    //Enable text input
    //SDL_StartTextInput();

    //While application is running
    while( !quit )
    {
	//Handle events on queue
	while( SDL_PollEvent( &e ) != 0 )
	{
	    //User requests quit
	    if( e.type == SDL_QUIT )
	    {
		quit = true;
	    }
	    else if( e.type == SDL_KEYDOWN)
	    {
		HandleKeyDown(e.key.keysym);
	    }
	}

	//Render quad
	//render();
			
	//Update screen
	SDL_GL_SwapWindow( win );
    }
    //SDL_StopTextInput();
    SDL_DestroyWindow(win);
    SDL_Quit();


    return 0;

    return 0;
}

void HandleKeyDown(SDL_Keysym key)
{
    switch(key.sym)
    {
    case SDLK_ESCAPE:
	quit=true;
	break;
    }
}
