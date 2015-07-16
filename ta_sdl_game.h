
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


struct InputState
{
    uint32_t KeyIsDown[256];
    uint32_t KeyWasDown[256];
};

struct Memory
{
    uint64_t PermanentStoreSize;
    uint8_t * PermanentStore;
    uint64_t TransientStoreSize;
    uint8_t * TransientStore;
};

typedef uint64_t memory_index;

struct MemoryArena
{
    memory_index Size;
    uint8_t *Base;
    memory_index Used;

    int32_t TempCount;
};


inline void
InitializeArena(MemoryArena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8_t *)Base;
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
    if((uint64_t)Memory + Size != Arena->Used + (uint64_t)Arena->Base)
    {
	printf("NOPE, NOT ALLOWED TO POP THIS!");
	return;
    }
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
    if((uint64_t)Memory + Size != Arena->Used + (uint64_t)Arena->Base)
    {
	printf("NOPE, NOT ALLOWED TO POP THIS!");
	return;
    }
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
    struct UnitDetails * Details;
    int Size;
};

struct GameState
{
    bool32 IsInitialised;
    MemoryArena GameArena;
    MemoryArena TempArena;
    bool32 Quit;


    struct Matrix * ProjectionMatrix;
    Matrix * ModelMatrix;
    Matrix * ViewMatrix;

    struct ShaderProgram * UnitShader;
    struct ShaderProgram * MapShader;
    struct ShaderProgram * FontShader;
    struct ShaderProgram * UIElementShaderProgram;    
    int64_t StartTime;
    int NumberOfFrames;
    GLuint ProjectionMatrixLocation;
    GLuint ModelMatrixLocation;
    GLuint ViewMatrixLocation;
    GLuint FontPositionLocation,FontColorLocation;
    GLuint UIElementRenderingVertexBuffer;

    GLuint UIElementPositionLocation,UIElementSizeLocation,UIElementColorLocation,UIElementBorderColorLocation,UIElementBorderWidthLocation,UIElementAlphaLocation;

    struct UIElement * TestElement;
    struct Object3d * temp_model;
    struct TAMap * TestMap;
    struct Object3dTransformationDetails * UnitTransformationDetails;
    int UnitIndex;

    TempUnitList Units;

    int ScreenWidth,ScreenHeight;

    struct HPIFileCollection * GlobalArchiveCollection;
    
    struct TextureContainer * UnitTextures;
    struct TextureContainer * Font11;
    struct TextureContainer * Font12;

    uint8_t * PaletteData;
    bool32 PaletteLoaded;
    uint8_t * FontBitmap;

    struct ScreenText * NameAndDescText;
    struct ScreenText * UnitDetailsText;
    
    struct FontDetails * Times32;
    struct FontDetails * Times24;
    struct FontDetails * Times16;

    struct UnitScript * CurrentUnitScript;
    uint64_t PerformanceCounterStart;
    uint64_t PerformanceCounterFrequency;

    float CameraXRotation;
    float CameraYRotation;

    float CameraTranslation[3];
    struct ScriptStatePool * CurrentScriptPool;
    struct UIElement * ScriptBackground;
    GLuint DebugAxisBuffer;
    struct FontShaderDetails * FontShaderDetails;
    struct FNTFont * Fonts;
    GLuint Draw2DVertexBuffer;
    struct Texture2DShaderDetails * DrawTextureShaderDetails;
    

    struct ShaderProgram * Shaders;
    int NumberOfShaders;
    struct TextureContainer * CommonGUITextures;
    struct TAUIElement * MainGUI;
};

const int UNIT_TEXTURE_WIDTH=2048;
const int UNIT_TEXTURE_HEIGHT=1024;
const int UNIT_MAX_TEXTURES=1024;

const int COMMONUI_TEXTURE_WIDTH=4096;
const int COMMONUI_TEXTURE_HEIGHT=1024;
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
