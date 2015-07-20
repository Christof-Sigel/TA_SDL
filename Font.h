
struct FNTGlyph
{
    int Width;
    float U;
    float TextureWidth;
};

struct FNTFont
{
    int Height;
    GLuint Texture;
    FNTGlyph Characters[256];
};

struct FontShaderDetails
{
    GLuint ColorLocation, AlphaLocation, PositionLocation, SizeLocation, TextureOffsetLocation;
    ShaderProgram * Program;
};
