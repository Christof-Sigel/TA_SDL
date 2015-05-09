
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
