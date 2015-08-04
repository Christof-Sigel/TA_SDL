
//TODO(Christof): Proper snprintf wrapper for MSVC
//NOTE(Christof): This will only work for the specific cases we use it for, i.e. calc length with 0,NULL
// _snprintf return value for buffer size too small IS NOT WHAT WE EXPECT!!!
#ifdef _MSC_VER
#define snprintf _snprintf
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#else
#define Assert(Expression) if(!(Expression)) {__builtin_trap();}
#endif
#include <stdio.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t  s16;
typedef int8_t  s8;


typedef uint64_t  u64;
typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t  u8;

typedef s64 b64;
typedef s32 b32;
typedef s16 b16;
typedef s8 b8;

typedef float r32;
typedef double r64;

const int UNIT_TEXTURE_WIDTH=1024;
const int UNIT_TEXTURE_HEIGHT=1400;
const int UNIT_MAX_TEXTURES=1024;

const int COMMONUI_TEXTURE_WIDTH=1024;
const int COMMONUI_TEXTURE_HEIGHT=1400;
const int COMMONUI_MAX_TEXTURES=1024;


const float TA_TO_GL_SCALE=1.0f/168340.0f;
const float PI = 3.14159265358979323846;
const int FONT_BITMAP_SIZE=256;

const int NUMBER_OF_UNIT_DETAILS=35;

const float COB_LINEAR_CONSTANT = 168340;
const float COB_LINEAR_FRAME_CONSTANT = COB_LINEAR_CONSTANT *2.0f  ;
const float COB_ANGULAR_CONSTANT = 182.044444f*180.0f/PI;
const float COB_ANGULAR_FRAME_CONSTANT = COB_ANGULAR_CONSTANT *2.0f ;

const int MAX_TA_FONT_NUMBER = 32;
const int MAX_SHADER_NUMBER = 32;
const int MAX_SHADER_FILENAME = 50;
#include "platform_code.cpp"
#include "GL.h"
#include "file_formats.h"


struct InputState
{
    u32 KeyIsDown[256];
    u32 KeyWasDown[256];
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

    s32 TempCount;
};


inline void
InitializeArena(MemoryArena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}



#define VERBOSE_ALLOCATIONS 0

#if VERBOSE_ALLOCATIONS
#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type),__func__,__LINE__,__FILE__)
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type),__func__,__LINE__,__FILE__)
#define PushSize(Arena, Size) PushSize_(Arena, Size,__func__,__LINE__,__FILE__)
#define PopArray(Arena, Memory, Count, type) PopSize_(Arena,Memory,(Count)*sizeof(type),__func__,__LINE__,__FILE__)
#define PushSubArena(Arena, Size) PushSubArena_(Arena,Size,__func__,__LINE__,__FILE__)
#define PopSubArena(Arena, SubArena) PopSubArena_(Arena, SubArena,__func__,__LINE__,__FILE__)

inline void *
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

MemoryArena * PushSubArena_(MemoryArena * Arena, memory_index Size, const char * caller, int line, const char * file)
{
    MemoryArena * Result = (MemoryArena*)PushSize_(Arena, Size+sizeof(MemoryArena),  caller,  line, file);
    InitializeArena(Result, Size, Result+sizeof(MemoryArena));
    return Result;
}


inline void PopSize_(MemoryArena * Arena, void * Memory, memory_index Size, const char * caller, int line, const char * file)
{
    printf("Popping %d from %s in %s:%d\n",Size,caller, file,line);
    Assert(((u64 )Memory + Size == Arena->Used + (us64 )Arena->Base);
    Arena->Used -=Size;
}

void PopSubArena_(MemoryArena * Arena, MemoryArena * SubArena ,const char * caller, int line, const char * file)
{
    PopSize_(Arena, SubArena, SubArena->Size+sizeof(MemoryArena), caller,  line, file);
}


#else

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, Size) PushSize_(Arena, Size)
#define PopArray(Arena, Memory, Count, type) PopSize_(Arena,Memory,(Count)*sizeof(type))

inline void *
PushSize_(MemoryArena *Arena, memory_index Size)
{
    if(Size ==0)
	return 0;
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

MemoryArena * PushSubArena(MemoryArena * Arena, memory_index Size)
{
    MemoryArena * Result = (MemoryArena*)PushSize_(Arena, Size+sizeof(MemoryArena));
    InitializeArena(Result, Size, Result+sizeof(MemoryArena));
    return Result;
}

inline void PopSize_(MemoryArena * Arena, void * Memory, memory_index Size)
{
    Assert((u64 )Memory + Size == Arena->Used + (u64 )Arena->Base);

    Arena->Used -=Size;
}

void PopSubArena(MemoryArena * Arena, MemoryArena * SubArena)
{
    PopSize_(Arena, SubArena, SubArena->Size+sizeof(MemoryArena));
}


#endif
#define MAX_UNITS_LOADED 1024

struct TempUnitList
{
    UnitDetails Details[MAX_UNITS_LOADED];
    int Size;
};

struct GameState
{
    b32 IsInitialised;
    MemoryArena GameArena;
    MemoryArena TempArena;
    b32 Quit;


    Matrix ProjectionMatrix;
    Matrix ModelMatrix;
    Matrix ViewMatrix;

    ShaderProgram * UnitShader;
    ShaderProgram * MapShader;
    ShaderProgram * UIElementShaderProgram;
    
    s64  StartTime;
    int NumberOfFrames;
    GLuint ProjectionMatrixLocation;
    GLuint ModelMatrixLocation;
    GLuint ViewMatrixLocation;
    GLuint UIElementRenderingVertexBuffer;

    GLuint UIElementPositionLocation,UIElementSizeLocation,UIElementColorLocation,UIElementBorderColorLocation,UIElementBorderWidthLocation,UIElementAlphaLocation;

    Object3d temp_model;
    TAMap TestMap;
    Object3dTransformationDetails UnitTransformationDetails;
    int UnitIndex;

    TempUnitList Units;

    int ScreenWidth,ScreenHeight;
    SDL_Window * MainSDLWindow;

    HPIFileCollection GlobalArchiveCollection;
    
    TextureContainer UnitTextures;
    TextureContainer Font11;
    TextureContainer Font12;

    u8 PaletteData[1024];

    UnitScript CurrentUnitScript;
    u64  PerformanceCounterStart;
    u64  PerformanceCounterFrequency;

    float CameraXRotation;
    float CameraYRotation;

    float CameraTranslation[3];
    ScriptStatePool CurrentScriptPool;
    GLuint DebugAxisBuffer;
    FontShaderDetails FontShaderDetails;
    FontContainer LoadedFonts;
    GLuint Draw2DVertexBuffer;
    Texture2DShaderDetails DrawTextureShaderDetails;


    ShaderGroup ShaderGroup;
    TextureContainer CommonGUITextures;
    TAUIElement *GUIs;
    int NumberOfGuis;
};

