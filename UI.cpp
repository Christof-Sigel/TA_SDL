
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





bool32 LoadFont(FontDetails * Font,const char * File, float Height,uint8_t * FontBitmap)
{
    MemoryMappedFile FontFile = MemoryMapFile(File);
    if(!FontFile.MMapBuffer)
    {
	LogError("Could not load font file %s",File);
	return 0;
    }
    int FirstUnused=stbtt_BakeFontBitmap(FontFile.MMapBuffer,0, Height, FontBitmap,FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, 32,96, Font->FontData); // no guarantee this fits!
    Font->Height=Height;
    UnMapFile(FontFile);
    if(FirstUnused<=0)
    {
	LogError("Only %d characters of font %s fit",-FirstUnused,File);
	return 0;
    }
    
    glGenTextures(1, &Font->FontTexture);
    glBindTexture(GL_TEXTURE_2D, Font->FontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, FontBitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return 1;
}



float TextWidth(char * Text, FontDetails * Font)
{
    float Width=0,Y=0;
    while(*Text)
    {
	if (*Text >= 32 && *Text >0) {
	    stbtt_aligned_quad q;
	    stbtt_GetBakedQuad(Font->FontData, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, *Text-32, &Width,&Y,&q,1);
	}
	Text++;
    }
    return Width;
}



ScreenText SetupOnScreenText(char * Text, float X, float Y,float Red, float Green, float Blue,FontDetails * Font,
			     float MaxWidth=-1)
{
    if(MaxWidth==-1)
	MaxWidth=TextWidth(Text,Font);
    int NumQuads=0;
    char * t=Text;
    while (*t) {
	if (*t >= 32 && *t >0) {
	    NumQuads++;
	}
	t++;
    }
    const int NUM_FLOATS_PER_QUAD=2*3*2*2;//2 triangles per quad, 3 verts per triangle, 2 position and 2 texture coords per vert
    //GLfloat * VertexAndTexCoordData=alloca[NumQuads*NUM_FLOATS_PER_QUAD];
    STACK_ARRAY(VertexAndTexCoordData,NumQuads*NUM_FLOATS_PER_QUAD,GLfloat);
    ScreenText result;
    result.Font=Font;
    result.Position.X=X;
    result.Position.Y=Y;
    result.Color.Red=Red;
    result.Color.Blue=Blue;
    result.Color.Green=Green;
    glGenVertexArrays(1,&result.VertexArrayObject);
    float TextX=0,TextY=0;
    for(int i=0;i<NumQuads;i++)
    {
	if (Text[i] >= 32 && Text[i] >0) {
	    stbtt_aligned_quad q;
	    //TODO(Christof): Make this position independant
	    stbtt_GetBakedQuad(Font->FontData, FONT_BITMAP_SIZE,FONT_BITMAP_SIZE, Text[i]-32, &TextX,&TextY,&q,1);
	    if(TextX>MaxWidth)
	    {
		NumQuads = i;
		break;
	    }
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 0]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 1]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 2]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 3]=q.t0;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 4]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 5]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 6]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 7]=q.t1;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 8]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 9]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 10]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 11]=q.t1;


	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 12]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 13]=q.y1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 14]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 15]=q.t1;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 16]=q.x1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 17]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 18]=q.s1;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 19]=q.t0;

	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 20]=q.x0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 21]=q.y0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 22]=q.s0;
	    VertexAndTexCoordData[i*NUM_FLOATS_PER_QUAD + 23]=q.t0;

	    result.Width=q.x1;
	}
    }

    result.NumberOfVertices=NumQuads * 6;

    glBindVertexArray(result.VertexArrayObject);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*NumQuads*NUM_FLOATS_PER_QUAD,VertexAndTexCoordData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,(GLvoid*)(sizeof(GLfloat)*2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
//    glDeleteBuffers(1,&VertexBuffer);
    
    return result;
}

void DrawOnScreenText(ScreenText Text, ShaderProgram * FontShader, GLuint FontPositionLocation, GLuint FontColorLocation)
{
    if(!FontShader)
	return;
    if(!Text.Font)
	return;
    glUseProgram(FontShader->ProgramID);
    glUniform2fv(FontPositionLocation,1,Text.Position.Contents);
    glUniform3fv(FontColorLocation,1,Text.Color.Contents);
    glBindVertexArray(Text.VertexArrayObject);
    glBindTexture(GL_TEXTURE_2D,Text.Font->FontTexture);
    glDrawArrays(GL_TRIANGLES,0,Text.NumberOfVertices);
}


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


void SetupUIElementDrawing(GameState * CurrentGameState)
{
    GLfloat RenderData[6*(2+4)];//6 Vert (2 triangles) each 2 position coords and 4 distance to edge "coords"
    
    glGenVertexArrays(1,&CurrentGameState->UIElementRenderingVertexBuffer);

    GLfloat Vertices[]={0,0, 1,0, 1,1, 0,1};
    GLfloat EdgeDistance[]={0,1,1,0, 0,0,1,1, 1,0,0,1, 1,1,0,0};

    int Indexes1[]={0,3,1};
    for(int i=0;i<3;i++)
    {
	RenderData[i*(2+4)+0]=Vertices[Indexes1[i]*2+0];
	RenderData[i*(2+4)+1]=Vertices[Indexes1[i]*2+1];

	RenderData[i*(2+4)+2]=EdgeDistance[Indexes1[i]*4+0];
	RenderData[i*(2+4)+3]=EdgeDistance[Indexes1[i]*4+1];
	RenderData[i*(2+4)+4]=EdgeDistance[Indexes1[i]*4+2];
	RenderData[i*(2+4)+5]=EdgeDistance[Indexes1[i]*4+3];
    }

    int Indexes2[]={1,3,2};

    for(int i=0;i<3;i++)
    {
	RenderData[6*3+i*(2+4)+0]=Vertices[Indexes2[i]*2+0];
	RenderData[6*3+i*(2+4)+1]=Vertices[Indexes2[i]*2+1];

	RenderData[6*3+i*(2+4)+2]=EdgeDistance[Indexes2[i]*4+0];
	RenderData[6*3+i*(2+4)+3]=EdgeDistance[Indexes2[i]*4+1];
	RenderData[6*3+i*(2+4)+4]=EdgeDistance[Indexes2[i]*4+2];
	RenderData[6*3+i*(2+4)+5]=EdgeDistance[Indexes2[i]*4+3];
    }
		      
    
    glBindVertexArray(CurrentGameState->UIElementRenderingVertexBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);


    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*6*(2+4),RenderData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,0);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,(GLvoid*)(sizeof(GLfloat)*2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    //glDeleteBuffers(1,&VertexBuffer);



}


UIElement SetupUIElement(float X, float Y, float Width, float Height, float BackgroundRed, float BackgroundGreen, float BackgroundBlue, float BorderRed, float BorderGreen, float BorderBlue, float BorderWidth, float Alpha)
{
    //TODO(Christof): Add text rendering
    UIElement result={{{0}}};

    result.Position.X=X;
    result.Position.Y=Y;
    result.Size.Width=Width;
    result.Size.Height=Height;
    result.BackgroundColor.Red=BackgroundRed;
    result.BackgroundColor.Blue=BackgroundBlue;
    result.BackgroundColor.Green=BackgroundGreen;

    result.BorderColor.Red=BorderRed;
    result.BorderColor.Blue=BorderBlue;
    result.BorderColor.Green=BorderGreen;

    result.BorderWidth=BorderWidth;
    result.Alpha = Alpha;
    
    return result;
}

UIElement SetupUIElementEnclosingText(float X, float Y,float BackgroundRed, float BackgroundGreen, float BackgroundBlue, float BorderRed, float BorderGreen, float BorderBlue, float BorderWidth, float Alpha, int NumberOfTexts, ScreenText* Texts, float TopPadding=0, float BottomPadding=5,float LeftPadding=5,float RightPadding=5)
{
    float Width=0;
    float Height=TopPadding;
    for(int i=0;i< NumberOfTexts;i++)
    {
	if(Width<Texts[i].Width)
	    Width=Texts[i].Width;
	Height+=Texts[i].Font->Height;
	Texts[i].Position.X=X+LeftPadding+BorderWidth;
	Texts[i].Position.Y=Y+BorderWidth+Height;
    }
    Height+=BottomPadding;
    Width+=LeftPadding+RightPadding;
    UIElement Result=SetupUIElement(X,Y,Width+BorderWidth*2,Height+BorderWidth*2,BackgroundRed,BackgroundGreen,BackgroundBlue,BorderRed,BorderGreen,BorderBlue,BorderWidth,Alpha);
    Result.Texts=Texts;
    Result.NumberOfTexts=NumberOfTexts;
    return Result;
}


void DrawUIElement(UIElement Element,ShaderProgram * UIElementShaderProgram,GLuint UIElementPositionLocation, GLuint UIElementSizeLocation, GLuint UIElementColorLocation,GLuint UIElementBorderColorLocation,GLuint UIElementBorderWidthLocation, GLuint UIElementAlphaLocation, GLuint UIElementRenderingVertexBuffer, ShaderProgram * FontShader, GLuint FontPositionLocation, GLuint FontColorLocation)
{
    glUseProgram(UIElementShaderProgram->ProgramID);
    glUniform2fv(UIElementPositionLocation,1,Element.Position.Contents);
    glUniform2fv(UIElementSizeLocation,1,Element.Size.Contents);
    glUniform3fv(UIElementColorLocation,1,Element.BackgroundColor.Contents);
    glUniform3fv(UIElementBorderColorLocation,1,Element.BorderColor.Contents);
    glUniform1f(UIElementBorderWidthLocation,Element.BorderWidth);
    glUniform1f(UIElementAlphaLocation,Element.Alpha);
    glBindVertexArray(UIElementRenderingVertexBuffer);
    glDrawArrays(GL_TRIANGLES,0,6);
    for(int i=0;i<Element.NumberOfTexts;i++)
    {
	DrawOnScreenText(Element.Texts[i], FontShader, FontPositionLocation, FontColorLocation);
    }
}

