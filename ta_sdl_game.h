
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

inline void *
PushSize_(MemoryArena *Arena, memory_index Size, const char * caller, int line, const char * file)
{
    //Assert((Arena->Used + Size) <= Arena->Size);
    printf("Allocating %d from %s in %s:%d\n",Size,caller, file,line);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#else

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, Size) PushSize_(Arena, Size)

inline void *
PushSize_(MemoryArena *Arena, memory_index Size)
{
    //Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#endif

#include <vector>

struct GameState
{
    bool32 IsInitialised;
    MemoryArena GameArena;
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
    int UnitIndex;

    std::vector<struct UnitDetails> Units;

    int ScreenWidth,ScreenHeight;

    struct HPIFileCollection * GlobalArchiveCollection;
    
    struct Texture * Textures;
    int NextTexture;
    uint8_t * TextureData;

    uint8_t * PaletteData;
    bool32 PaletteLoaded;
    GLuint UnitTexture;
    uint8_t * FontBitmap;

};

const int TEXTURE_WIDTH=2048;
const int TEXTURE_HEIGHT=2048;
const int MAX_NUMBER_OF_TEXTURE=1024;


const double TA_TO_GL_SCALE=1.0f/2500000.0f;
const double PI = 3.14159265358979323846;
const int FONT_BITMAP_SIZE=256;
