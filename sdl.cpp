



void HandleKeyDown(SDL_Keysym key, InputState * GameInputState)
{
    uint8_t Index = key.sym&255;
    GameInputState->KeyWasDown[Index]=GameInputState->KeyIsDown[Index];
    GameInputState->KeyIsDown[Index]=1;
}

void HandleKeyUp(SDL_Keysym key, InputState * GameInputState)
{
    uint8_t Index = key.sym & 255;
    GameInputState->KeyWasDown[Index]=GameInputState->KeyIsDown[Index];
    GameInputState->KeyIsDown[Index]=0;
}


SDL_Window * MainSDLWindow;
const int DEFAULT_SCREEN_WIDTH=1280;
const int DEFAULT_SCREEN_HEIGHT=1024;

bool32 SetupSDLWindow(GameState * CurrentGameState)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
	LogError("SDL_Init Error: %s", SDL_GetError());
	return 0;
    }

    if(CurrentGameState->ScreenWidth == 0)
    {
	CurrentGameState->ScreenWidth = DEFAULT_SCREEN_WIDTH;
    }
    if(CurrentGameState->ScreenHeight == 0)
    {
	CurrentGameState->ScreenHeight = DEFAULT_SCREEN_HEIGHT;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); 

    MainSDLWindow = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,CurrentGameState->ScreenWidth, CurrentGameState->ScreenHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    if (!MainSDLWindow)
    {
	LogError("SDL_CreateWindow Error: %s", SDL_GetError());
	return 0;
    }


    GLint GLMajorVer, GLMinorVer;
    
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed before create context : %s", gluErrorString(ErrorValue));
    }
    

    SDL_GLContext gContext = SDL_GL_CreateContext( MainSDLWindow);
    SDL_GL_MakeCurrent (MainSDLWindow,gContext);
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
	LogWarning("Warning: Unable to set VSync! SDL Error: %s",SDL_GetError());
    }

    glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVer);
    glGetIntegerv(GL_MINOR_VERSION, &GLMinorVer);
    LogDebug("OpenGL Version %d.%d",GLMajorVer,GLMinorVer);


    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed before glewInit : %s",gluErrorString(ErrorValue));
    }
  

    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
	LogError("Error initializing GLEW! %s", glewGetErrorString( glewError ));
	return 0;
    }

    
    //as we need to use glewExperimental - known issue (it segfaults otherwise!) - we encounter
    //another known issue, which is that while glewInit suceeds, it leaves opengl in an error state
    ErrorValue = glGetError();

    return 1;
}


