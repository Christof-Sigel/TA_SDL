#pragma pack(push,1)


struct FILE_FNT
{
    uint8_t Height;
    uint8_t Padding1[3];
    int16_t CharacterOffset[255];
};

#pragma pack(pop)

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


union FontColor
{
    struct
    {
	float Red,Green,Blue;
    };
    float Contents[3];
};

struct FontShaderDetails
{
    GLuint ColorLocation, AlphaLocation, PositionLocation, SizeLocation, TextureOffsetLocation;
    ShaderProgram Program;
};

void LoadCharacter(int16_t CharacterOffset, uint8_t * FileBuffer, uint8_t * TextureBuffer, int XOffset, int YOffset, int Height, int TextureWidth)
{
    if(CharacterOffset ==0)
	return;
    int Width = * (FileBuffer + CharacterOffset );
    if(Width == 0) 
	return;
    uint8_t * ImageData = (FileBuffer + CharacterOffset + 1);
    int ByteOffset = 0, X =0, Y=0;
    while(Y< Height)
    {
	for(int i=7;i>=0;i--)
	{
	    int bit = ImageData[ByteOffset]&(1<<i);
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+0] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+1] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+2] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+3] = bit*255;
	    X++;
	    if(X>=Width)
	    {
		X=0;
		Y++;
		if(Y>=Height)
		    break;
	    }
	}
	ByteOffset++;
    }
}


void LoadFNTFont(uint8_t * Buffer, FNTFont * Font, int Size, GameState * CurrentGameState)
{
    FILE_FNT * Header = (FILE_FNT*)Buffer;
    GLuint FontTexture;
    Font->Height = Header->Height;
    int TextureWidth = 0;

    for(int i=0;i<255;i++)
    {
	Font->Characters[i] = {};
	if( Header->CharacterOffset[i])
	{
	    Font->Characters[i].Width= *( Buffer + Header->CharacterOffset[i]);
	    TextureWidth += Font->Characters[i].Width;
	}
    }

    STACK_ARRAY(TextureData, TextureWidth*Font->Height*4, uint8_t);
    int XOffset = 0;
    for(int i=0;i<254;i++)
    {
	LoadCharacter(Header->CharacterOffset[i] ,Buffer, TextureData, XOffset, 0, Header->Height, TextureWidth);
	Font->Characters[i].U=XOffset/(float)TextureWidth;
	Font->Characters[i].TextureWidth = Font->Characters[i].Width/(float)TextureWidth;
	XOffset += Font->Characters[i].Width;
    }
    glGenTextures(1,&Font->Texture);
    glBindTexture(GL_TEXTURE_2D,Font->Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TextureWidth,Font->Height,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);

}


void DrawCharacter(char Character, FontShaderDetails * ShaderDetails, GLuint VertexBuffer , float X, float Y, FontColor Color, float Alpha, FNTFont * Font )
{
    glUseProgram(ShaderDetails->Program.ProgramID);
    glUniform3fv(ShaderDetails->ColorLocation,1,Color.Contents);
    glUniform2f(ShaderDetails->PositionLocation,X,Y);
    glUniform1f(ShaderDetails->AlphaLocation,Alpha);
    glUniform2f(ShaderDetails->TextureOffsetLocation,Font->Characters[Character].U, Font->Characters[Character].TextureWidth);
    glUniform2f(ShaderDetails->SizeLocation,Font->Characters[Character].Width, Font->Height);

    

		 
    glBindVertexArray(VertexBuffer);
    glBindTexture(GL_TEXTURE_2D,Font->Texture);
    glDrawArrays(GL_TRIANGLES,0,6);
}


void SetupFontRendering(GameState * CurrentGameState)
{
    GLfloat RenderData[6*(2+2)];//6 Vert (2 triangles) each 2 position coords and 2 texture coords
    
    glGenVertexArrays(1,&CurrentGameState->FontVertexBuffer);

    GLfloat Vertices[]={0,0, 1,0, 1,1, 0,1};

    int Indexes1[]={0,3,1};
    for(int i=0;i<3;i++)
    {
	RenderData[i*(2+2)+0]=Vertices[Indexes1[i]*2+0];
	RenderData[i*(2+2)+1]=Vertices[Indexes1[i]*2+1];

	RenderData[i*(2+2)+2]=Vertices[Indexes1[i]*2+0];
	RenderData[i*(2+2)+3]=Vertices[Indexes1[i]*2+1];

    }

    int Indexes2[]={1,3,2};

    for(int i=0;i<3;i++)
    {
	RenderData[(i+3)*(2+2)+0]=Vertices[Indexes2[i]*2+0];
	RenderData[(i+3)*(2+2)+1]=Vertices[Indexes2[i]*2+1];

	RenderData[(i+3)*(2+2)+2]=Vertices[Indexes2[i]*2+0];
	RenderData[(i+3)*(2+2)+3]=Vertices[Indexes2[i]*2+1];
    }
		      
    
    glBindVertexArray(CurrentGameState->FontVertexBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);


    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*6*(2+2),RenderData,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,(void*)(sizeof(GLfloat)*2));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
