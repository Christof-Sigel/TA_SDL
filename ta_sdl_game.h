//TODO(Christof): Proper snprintf wrapper for MSVC
//NOTE(Christof): This will only work for the specific cases we use it for, i.e. calc length with 0,NULL
// _snprintf return value for buffer size too small IS NOT WHAT WE EXPECT!!!
#ifdef _MSC_VER

#define Assert(Expression) if(!(Expression)) {*(s32*)0 = 0;}
#else
#define Assert(Expression) if(!(Expression)) {__builtin_trap();}
#endif
#include <stdio.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef s64 b64;
typedef s32 b32;
typedef s16 b16;
typedef s8 b8;

typedef float r32;
typedef double r64;

#define internal static

const int UNIT_TEXTURE_WIDTH=1024;
const int UNIT_TEXTURE_HEIGHT=1400;
const int UNIT_MAX_TEXTURES=1024;

const int COMMONUI_TEXTURE_WIDTH=1024;
const int COMMONUI_TEXTURE_HEIGHT=1400;
const int COMMONUI_MAX_TEXTURES=1024;


const float TA_TO_GL_SCALE=1.0f/168340.0f;
const float PI = 3.14159265358979323846f;
const int FONT_BITMAP_SIZE=256;

const int NUMBER_OF_UNIT_DETAILS=35;

const float COB_LINEAR_CONSTANT = 168340;
const float COB_LINEAR_FRAME_CONSTANT = COB_LINEAR_CONSTANT  ;
const float COB_ANGULAR_CONSTANT = 182.044444f*180.0f/PI;
const float COB_ANGULAR_FRAME_CONSTANT = COB_ANGULAR_CONSTANT ;


const int MAX_TA_FONT_NUMBER = 32;
const int MAX_SHADER_NUMBER = 32;
const int MAX_SHADER_FILENAME = 50;
#include "Logging.h"
#include "GL.h"
#include "file_formats.h"

struct Unit
{
    UnitScript * Script;
    ScriptStatePool ScriptPool;
    char * Name;
    UnitDetails * Details;
    UnitSide Side;
    r32 Health, BuildPercent;
    s32 PAD;
};


struct InputState
{
    u32 KeyIsDown[256];
    u32 KeyWasDown[256];
    u32 MouseButtons;
    u32 LastMouseButtons;
    s32 MouseX, MouseY;
};

struct Memory
{
    u64  PermanentStoreSize;
    u8 * PermanentStore;
    u64  TransientStoreSize;
    u8 * TransientStore;
};

typedef u64  memory_index;

struct MemoryArena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

inline void
InitializeArena(MemoryArena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
}

#define VERBOSE_ALLOCATIONS 0

#if VERBOSE_ALLOCATIONS
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type),__func__,__LINE__,__FILE__)
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type),__func__,__LINE__,__FILE__)
#define PushSize(Arena, Size) PushSize_(Arena, Size,__func__,__LINE__,__FILE__)
#define PopArray(Arena, Memory, Count, type) PopSize_(Arena,Memory,(Count)*sizeof(type),__func__,__LINE__,__FILE__)
#define PushSubArena(Arena, Size) PushSubArena_(Arena,Size,__func__,__LINE__,__FILE__)
#define PopSubArena(Arena, SubArena) PopSubArena_(Arena, SubArena,__func__,__LINE__,__FILE__)

internal inline void *
    PushSize_(MemoryArena *Arena, memory_index Size, const char * caller, int line, const char * file)
{
    if(Size ==0)
	return 0;
    Assert((Arena->Used + Size) <= Arena->Size);
    printf("Allocating %d from %s in %s:%d\n",Size,caller, file,line);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return(Result);
}

internal inline MemoryArena * PushSubArena_(MemoryArena * Arena, memory_index Size, const char * caller, int line, const char * file)
{
    MemoryArena * Result = (MemoryArena*)PushSize_(Arena, Size+sizeof(MemoryArena),  caller,  line, file);
    InitializeArena(Result, Size, Result+sizeof(MemoryArena));
    return Result;
}


internal inline void PopSize_(MemoryArena * Arena, void * Memory, memory_index Size, const char * caller, int line, const char * file)
{
    printf("Popping %d from %s in %s:%d\n",Size,caller, file,line);
    Assert(((u64 )Memory + Size == Arena->Used + (us64 )Arena->Base);
    Arena->Used -=Size;
}

internal inline void PopSubArena_(MemoryArena * Arena, MemoryArena * SubArena ,const char * caller, int line, const char * file)
{
    PopSize_(Arena, SubArena, SubArena->Size+sizeof(MemoryArena), caller,  line, file);
}


#else

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (u64)(Count)*sizeof(type))
#define PushSize(Arena, Size) PushSize_(Arena, Size)
#define PopArray(Arena, Memory, Count, type) PopSize_(Arena,Memory,(u64)(Count)*sizeof(type))

internal inline void *
PushSize_(MemoryArena *Arena, memory_index Size)
{
    if(Size ==0)
	return 0;
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return(Result);
}

internal inline MemoryArena * PushSubArena(MemoryArena * Arena, memory_index Size)
{
    MemoryArena * Result = (MemoryArena*)PushSize_(Arena, Size+sizeof(MemoryArena));
    InitializeArena(Result, Size, Result+sizeof(MemoryArena));
    return Result;
}

internal inline void PopSize_(MemoryArena * Arena, void * Memory, memory_index Size)
{
    Assert((u64 )Memory + Size == Arena->Used + (u64 )Arena->Base);

    Arena->Used -=Size;
}

internal inline void PopSubArena(MemoryArena * Arena, MemoryArena * SubArena)
{
    PopSize_(Arena, SubArena, SubArena->Size+sizeof(MemoryArena));
}


#endif


struct UnitShaderDetails
{
    ShaderProgram * Shader;
    GLint ProjectionMatrixLocation;
    GLint ModelMatrixLocation;
    GLint ViewMatrixLocation;
    s32 PAD;
};

struct UnitBuildShaderDetails
{
    ShaderProgram * Shader;
    GLint ProjectionMatrixLocation;
    GLint ModelMatrixLocation;
    GLint ViewMatrixLocation;
    GLint BuildPercentLocation;
};

struct DebugRectShaderDetails
{
    ShaderProgram * Program;
    GLint PositionLocation, SizeLocation;
    GLint BorderColorLocation, ColorLocation, BorderWidthLocation;
    GLuint VertexBuffer;
};

enum State
{
    STATE_MAIN_MENU,
    STATE_SINGLEPLAYER_MENU,
    STATE_OPTIONS_MENU,
    STATE_CAMPAIGN_MENU,
    STATE_LOAD_GAME_MENU,
    STATE_SKIRMISH_MENU,


    STATE_RUNNING,
    STATE_PAUSED,
    STATE_QUIT,
};

struct GameState
{
    UnitShaderDetails UnitShaderDetails;
    UnitBuildShaderDetails UnitBuildShaderDetails;
    ShaderProgram * MapShader;
    FontShaderDetails FontShaderDetails;
    Texture2DShaderDetails DrawTextureShaderDetails;
    ShaderGroup ShaderGroup;

    TextureContainer UnitTextures;
    TextureContainer Font11;
    TextureContainer Font12;
    TextureContainer CommonGUITextures;
    TextureContainer ArmInterfaceTextures;
    TextureContainer CoreInterfaceTextures;

    SDL_Window * MainSDLWindow;

    HPIFileCollection GlobalArchiveCollection;
    u8 PaletteData[1024];
    u64  PerformanceCounterFrequency;

    CampaignList CampaignList;
    MemoryArena GameArena;
    MemoryArena TempArena;
    State State;



    b32 IsInitialised;
    s32 NumberOfFrames;

    Matrix ProjectionMatrix;
    Matrix ModelMatrix;
    Matrix ViewMatrix;


    GLuint DebugAxisBuffer;
    DebugRectShaderDetails DebugRectDetails;
    GLuint DebugRectBuffer;

    FontContainer LoadedFonts;
    GLuint Draw2DVertexBuffer;

    int ScreenWidth,ScreenHeight;
    TAMap Map;


    //Test data

    TAUIElement MainMenu;
    TAUIElement SinglePlayerMenu;
    TAUIElement CampaignMenu;
    TAUIElement LoadGameMenu;
    TAUIElement SkirmishMenu;
    TAUIElement OptionsMenu;
    float CameraTranslation[3];

    float CameraXRotation;
    float CameraYRotation;





    s32 MouseX, MouseY;
    s32 PAD;
};
