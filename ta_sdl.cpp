#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>

typedef int32_t bool32;


#include "platform_code.cpp"
#include "file_formats.cpp"

void HandleKeyDown(SDL_Keysym key);
bool32 quit=0;
bool32 SetupSDLWindow();
void Render();
void Setup();
SDL_Window * MainSDLWindow;

int ScreenWidth=800,ScreenHeight=600;

int main(int argc, char * argv[])
{
    if(!SetupSDLWindow())
	return 1;

    Setup();
    
    SDL_Event e;
    while( !quit )
    {
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
	}
	Render();
	SDL_GL_SwapWindow( MainSDLWindow );
    }

    
    SDL_DestroyWindow(MainSDLWindow);
    SDL_Quit();

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

void PrintHPIDirectory(HPIDirectoryEntry dir, int Tabs=0)
{
    char printf_format[Tabs+2+1+1];
    for(int i=0;i<Tabs;i++)
	printf_format[i]='\t';
    printf_format[Tabs]='%';
    printf_format[Tabs+1]='s';
    printf_format[Tabs+2]='\n';
    printf_format[Tabs+3]=0;

    for(int EntryIndex=0;EntryIndex<dir.NumberOfEntries;EntryIndex++)
    {
	printf(printf_format,dir.Entries[EntryIndex].Name);
	if(dir.Entries[EntryIndex].IsDirectory)
	{
	    PrintHPIDirectory(dir.Entries[EntryIndex].Directory,Tabs+1);
	}
    }
}


void Setup()
{
    HPIFile main;
    LoadHPIFile("data/totala1.hpi",&main);
    PrintHPIDirectory(main.Root);

    
    HPIEntry Default = FindHPIEntry(main,"ai\\DEFAULT.TXT");
    if(Default.IsDirectory)
    {
	printf("%s is unexpectedly a directory!\n",Default.Name);
    }
    else if(Default.Name)
    {
	char temp[Default.File.FileSize+1];
	if(LoadHPIFileEntryData(Default,temp))
	{
	    temp[Default.File.FileSize]=0;
	    printf("%s\n",temp);
	}
    }
    else
    {
	printf("failed to find %s\n",Default.Name);
    }
}

void Render()
{

}

bool32 SetupSDLWindow()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	printf("SDL_Init Error: %s\n", SDL_GetError());
	return 0;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    MainSDLWindow = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    if (!MainSDLWindow)
    {
	printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
	return 0;
    }


    GLint GLMajorVer, GLMinorVer;
    
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	printf("failed before create context : %s\n", gluErrorString(ErrorValue));
    }
    

    SDL_GLContext gContext = SDL_GL_CreateContext( MainSDLWindow);
    SDL_GL_MakeCurrent (MainSDLWindow,gContext);
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
	return 0;
    }

    
    //as we need to use glewExperimental - known issue (it segfaults otherwise!) - we encounter
    //another known issue, which is that while glewInit suceeds, it leaves opengl in an error state
    ErrorValue = glGetError();

    return 1;
}
    
