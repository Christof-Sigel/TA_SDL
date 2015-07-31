
void LoadCharacter(s16  CharacterOffset, u8 * FileBuffer, u8 * TextureBuffer, int XOffset, int YOffset, int Height, int TextureWidth)
{
    if(CharacterOffset ==0)
	return;
    int Width = * (FileBuffer + CharacterOffset );
    if(Width == 0) 
	return;
    u8 * ImageData = (FileBuffer + CharacterOffset + 1);
    int ByteOffset = 0, X =0, Y=0;
    while(Y< Height)
    {
	for(int i=7;i>=0;i--)
	{
	    int bit = ImageData[ByteOffset]&(1<<i);
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+0] = 255;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+1] = 255;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+2] = 255;
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


void LoadFNTFont(u8 * Buffer, FNTFont * Font)
{
    FILE_FNT * Header = (FILE_FNT*)Buffer;
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

    STACK_ARRAY(TextureData, TextureWidth*Font->Height*4, u8 );
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


void DrawCharacter(u8 Character, Texture2DShaderDetails * ShaderDetails, float X, float Y, Color Color, float Alpha, FNTFont * Font )
{
    DrawTexture2D(Font->Texture, X,Y, Font->Characters[Character].Width, Font->Height, Color, Alpha, ShaderDetails, Font->Characters[Character].U, 0.0f,  Font->Characters[Character].TextureWidth, 1.0f);
}


void SetupFontRendering(GLuint * Draw2DVertexBuffer)
{
    GLfloat RenderData[6*(2+2)];//6 Vert (2 triangles) each 2 position coords and 2 texture coords
    
    glGenVertexArrays(1,Draw2DVertexBuffer);

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
		      
    
    glBindVertexArray(*Draw2DVertexBuffer);

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


void LoadGafFonts(GameState * CurrentGameState)
{
    SetupTextureContainer(&CurrentGameState->Font11, 1700, 15, 1, &CurrentGameState->GameArena);
    SetupTextureContainer(&CurrentGameState->Font12, 2200, 18, 1, &CurrentGameState->GameArena);
    
    HPIEntry Font = FindEntryInAllFiles("anims/hattfont12.GAF",&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
    if(Font.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load hatfont12.gaf");
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&Font, &CurrentGameState->Font12, &CurrentGameState->TempArena, CurrentGameState->PaletteData);
    }
    
    Font = FindEntryInAllFiles("anims/hattfont11.GAF", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
    if(Font.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load hatfont11.gaf");
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&Font, &CurrentGameState->Font11, &CurrentGameState->TempArena, CurrentGameState->PaletteData);
    }
}


void DrawBitmapCharacter(u8 Character, Texture2DShaderDetails * ShaderDetails, TextureContainer * TextureContainer, float X, float Y, Color Color, float Alpha, int * oWidth, int *oHeight)
{
    Texture tex = TextureContainer->Textures[0];
    float Width = tex.Widths[Character] * TextureContainer->TextureWidth;
    float Height = tex.Heights[Character] * TextureContainer->TextureHeight;
    *oWidth = (int)ceil(Width);
    *oHeight = (int)ceil(Height);
    float U = tex.U;
    float V = tex.V;
    for(int i=0;i<Character;i++)
    {
	U+= tex.Widths[i];
    }
    DrawTexture2D(TextureContainer->Texture, X, Y-Height, Width, Height, Color, Alpha, ShaderDetails, U, V, tex.Widths[Character], tex.Heights[Character]);
}


int TextWidthInPixels(char * Text, TextureContainer * Font)
{
    int Result =0;
    u8 * Char = (u8*)Text;
    while(*Char)
    {
	float CharWidth = Font->Textures[0].Widths[*Char];
	int CharWidthInPixels = CharWidth * Font->TextureWidth;
	Result += CharWidthInPixels;
	Char++;
    }
    return Result;
}

int TextHeightInPixels(char * Text, TextureContainer * Font)
{
    int Result =0;
    u8 * Char = (u8*)Text;
    while(*Char)
    {
	float CharHeight = Font->Textures[0].Heights[*Char];
	
	int CharHeightInPixels = CharHeight * Font->TextureHeight;
	if(Result<CharHeightInPixels)
	{
	Result = CharHeightInPixels;
	}
	Char++;
    }
    return Result;
}

void Draw2DFontText(char * Text, int X, int Y, TextureContainer * Font, Texture2DShaderDetails * ShaderDetails, int TextHeight)
{
    int FontHeightInPixels = Font->Textures[0].Heights[0]*Font->TextureHeight;
    u8* Char = (u8*)Text;
    while(*Char)
    {
	float CharWidth = Font->Textures[0].Widths[*Char];
	float CharHeight = Font->Textures[0].Heights[*Char];
	float U = Font->Textures[0].U, V = Font->Textures[0].V;
	for(int i=0;i<*Char;i++)
	{
	    U+=Font->Textures[0].Widths[i];
	}
	int CharWidthInPixels = CharWidth * Font->TextureWidth;
	int CharHeightInPixels = CharHeight * Font->TextureHeight;
	if(*Char > ' ')
	{
	    DrawTexture2D(Font->Texture, X, Y-Font->Textures[0].Y[*Char]+FontHeightInPixels, CharWidthInPixels, CharHeightInPixels, {{1,1,1}}, 1.0, ShaderDetails, U, V, CharWidth, CharHeight);
	}
	X+=CharWidthInPixels;
	Char++;
    }
}

FNTFont * GetFont(FontContainer * FontContainer, char * Name, HPIFileCollection * FileCollection, MemoryArena * TempArena)
{
    FNTFont * Result = 0;

    for(int i=0;i<FontContainer->NumberOfFonts;i++)
    {
	if(CaseInsensitiveMatch(FontContainer->FontNames[i],Name))
	{
	    return &FontContainer->Fonts[i];
	}
    }

    char FontName[74];
    
    int len=strlen(Name)+1;
    if(len>63)
	len=63;
   
    memcpy(&FontName[6], Name, len);
    FontName[len+9]=0;
    FontName[len+5]='.';
    FontName[len+6]='f';
    FontName[len+7]='n';
    FontName[len+8]='t';
    FontName[0]='f';
    FontName[1]='o';
    FontName[2]='n';
    FontName[3]='t';
    FontName[4]='s';
    FontName[5]='/';

    HPIEntry Font = FindEntryInAllFiles(FontName, FileCollection, TempArena);
    if(Font.Name)
    {
	if(Font.IsDirectory)
	{
	}
	else
	{
	    u8 * FontFileBuffer = PushArray(TempArena, Font.File.FileSize, u8);
	    LoadHPIFileEntryData(Font, FontFileBuffer, TempArena);
	    LoadFNTFont(FontFileBuffer, &FontContainer->Fonts[FontContainer->NumberOfFonts]);
	    PopArray(TempArena, FontFileBuffer, Font.File.FileSize, u8);
	    memcpy(FontContainer->FontNames[FontContainer->NumberOfFonts], Name, len);
	    FontContainer->FontNames[FontContainer->NumberOfFonts][len]=0;
	    return &FontContainer->Fonts[FontContainer->NumberOfFonts++];
	}
    }
    return 0;
}
