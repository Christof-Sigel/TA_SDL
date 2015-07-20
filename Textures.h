const int32_t GAF_IDVERSION=0x00010100;


struct TexturePosition
{
    int X;
    int Y;
};

#define MAX_NUMBER_OF_TEXTURE_FRAMES 256

struct Texture
{
    char Name[32];
    int NumberOfFrames;
    float U,V;
    float Widths[MAX_NUMBER_OF_TEXTURE_FRAMES],Heights[MAX_NUMBER_OF_TEXTURE_FRAMES];
    int X[MAX_NUMBER_OF_TEXTURE_FRAMES], Y[MAX_NUMBER_OF_TEXTURE_FRAMES];
};

struct TextureContainer
{
    Texture * Textures;
    int MaximumTextures;
    int NumberOfTextures;
    uint8_t * TextureData;
    int TextureWidth, TextureHeight;
    GLuint Texture;
    TexturePosition FirstFreeTexture;
};
