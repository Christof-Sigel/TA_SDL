internal void LoadCharacter(s16  CharacterOffset, u8 * FileBuffer, u8 * TextureBuffer, int XOffset, int YOffset, int Height, int TextureWidth)
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
	    u8 bit = ImageData[ByteOffset]&(1<<i);
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


internal void LoadFNTFont(u8 * Buffer, FNTFont * Font, MemoryArena * TempArena)
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

    u8 * TextureData  = PushArray(TempArena, TextureWidth*Font->Height*4, u8 );
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
    PopArray(TempArena, TextureData,TextureWidth*Font->Height*4, u8 );
}

internal void SetupFontRendering(GLuint * Draw2DVertexBuffer)
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


internal void LoadGafFonts(GameState * CurrentGameState)
{
    SetupTextureContainer(&CurrentGameState->Font11, 2120, 15, 256, &CurrentGameState->GameArena);
    SetupTextureContainer(&CurrentGameState->Font12, 2880, 18, 256, &CurrentGameState->GameArena);
    
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


internal void DrawBitmapCharacter(u8 Character, Texture2DShaderDetails * ShaderDetails, TextureContainer * TextureContainer, float X, float Y, Color Color, float Alpha, int * oWidth, int *oHeight)
{
    Texture * tex = GetTexture(&TextureContainer->Textures[0], Character);
    float Width = tex->Width * TextureContainer->TextureWidth;
    float Height = tex->Height * TextureContainer->TextureHeight;
    *oWidth = (int)ceil(Width);
    *oHeight = (int)ceil(Height);
    float U = tex->U;
    float V = tex->V;
    DrawTexture2D(TextureContainer->Texture, X, Y-Height, Width, Height, Color, Alpha, ShaderDetails, U, V, tex->Width, tex->Height);
}

internal int FontHeightInPixels(const char * Text, TextureContainer * Font)
{
    int Result =0;
    u8 * Char = (u8*)Text;
    while(*Char)
    {
	Texture * tex = GetTexture(&Font->Textures[0], *Char);
	float CharHeight = tex->Height;
	
	int CharHeightInPixels = int(CharHeight * Font->TextureHeight);
	if(Result<CharHeightInPixels)
	{
	    Result = CharHeightInPixels;
	}
	Char++;
    }
    return Result;
}

struct FontDimensions
{
    s32 Width;
    s32 Height;
};

internal FontDimensions TextSizeInPixels(const char * Text, TextureContainer * Font)
{
    FontDimensions Result = {};
    u8 * Char = (u8*)Text;
    Texture * Textures = &Font->Textures[0];
    int MaxHeight = 0;
    int CurrentWidth = 0;
    while(*Char)
    {
	float CharHeight = Textures[*Char].Height;
	int CharHeightInPixels = int(CharHeight * Font->TextureHeight);

	float CharWidth = Textures[*Char].Width;
	int CharWidthInPixels = int(CharWidth * Font->TextureWidth);
	if(*Char == '\n')
	{
	    Result.Height += MaxHeight;
	    if(Result.Width < CurrentWidth)
	    {
		Result.Width = CurrentWidth;
	    }
	    MaxHeight =0;
	    CurrentWidth=0;
	}
	else
	{
	    if(MaxHeight < CharHeightInPixels)
	    {
		MaxHeight = CharHeightInPixels;
	    }
	    CurrentWidth += CharWidthInPixels;
	}
	Char++;
    }
    if(CurrentWidth != 0 || MaxHeight != 0)
    {
	Result.Height += MaxHeight;
	if(Result.Width < CurrentWidth)
	{
	    Result.Width = CurrentWidth;
	}
    }
    return Result;
}

internal void DrawTextureFontText(const char * Text, int InitialX, int InitialY, TextureContainer * Font, Texture2DShaderDetails * ShaderDetails, float Alpha = 1.0, Color Color ={{1,1,1}})
{
    int TextHeight = FontHeightInPixels(Text, Font);
    u8* Char = (u8*)Text;
    int X =InitialX, Y=InitialY;
    while(*Char)
    {
	Texture * tex = GetTexture(&Font->Textures[0], *Char);
	float CharWidth = tex->Width;
	float CharHeight = tex->Height;
	float U = tex->U, V = tex->V;
	
	int CharWidthInPixels = int(CharWidth * Font->TextureWidth);
	int CharHeightInPixels = int(CharHeight * Font->TextureHeight);
	if(*Char > ' ')
	{
	    DrawTexture2D(Font->Texture,(float) X, float(Y-tex->Y+TextHeight), float(CharWidthInPixels), float(CharHeightInPixels), Color, Alpha, ShaderDetails, U, V, CharWidth, CharHeight);
	}
	if(*Char == '\n' || *Char == '\r')
	{
	    X=InitialX;
	    Y+=TextHeight;
	}
	else
	{
	    X+=CharWidthInPixels;
	}
	Char++;
    }
}

internal FNTFont * GetFont(FontContainer * FontContainer, char * Name, HPIFileCollection * FileCollection, MemoryArena * TempArena)
{
    for(int i=0;i<FontContainer->NumberOfFonts;i++)
    {
	if(CaseInsensitiveMatch(FontContainer->FontNames[i],Name))
	{
	    return &FontContainer->Fonts[i];
	}
    }

    char FontName[74];
    
    size_t len=strlen(Name)+1;
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
	    LoadFNTFont(FontFileBuffer, &FontContainer->Fonts[FontContainer->NumberOfFonts], TempArena);
	    PopArray(TempArena, FontFileBuffer, Font.File.FileSize, u8);
	    memcpy(FontContainer->FontNames[FontContainer->NumberOfFonts], Name, len);
	    FontContainer->FontNames[FontContainer->NumberOfFonts][len]=0;
	    return &FontContainer->Fonts[FontContainer->NumberOfFonts++];
	}
    }
    return 0;
}
