
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation

#include "libs/stb_truetype.h"

struct FontDetails
{
    float Height;
    stbtt_bakedchar FontData[96];
    GLuint FontTexture;
};

struct ScreenText
{
    GLuint VertexArrayObject;
    int NumberOfVertices;
    FontDetails * Font;
    float Width;
    union
    {
	struct
	{
	float X,Y;
	};
	float Contents[2];
    } Position;

    union
    {
	struct
	{
	    float Red,Green,Blue;
	};
	float Contents[3];
    } Color;
	
};



union UIElementPosition
{
    GLfloat Contents[2];
    
    struct{
	GLfloat X, Y;
    };
};

union UIElementSize
{
    GLfloat Contents[2];
    struct
    {
	GLfloat Width,Height;
    };
};

union UIElementColor
{
    GLfloat Contents[3];
    struct
    {
	GLfloat Red,Green,Blue;
    };
};

struct UIElement
{
    UIElementPosition Position;
    UIElementSize Size;
    UIElementColor BorderColor;
    UIElementColor BackgroundColor;
    GLfloat BorderWidth;
    GLfloat Alpha;
    int NumberOfTexts;
    ScreenText * Texts;
};
