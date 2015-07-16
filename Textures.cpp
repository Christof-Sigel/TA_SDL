

const int32_t GAF_IDVERSION=0x00010100;
#pragma pack(push,1)
struct FILE_GafHeader
{
    int32_t IDVersion;
    int32_t NumberOfEntries;
    int32_t Unknown1; //Always 0 apparently?
};

struct  FILE_GafEntry
{
    int16_t NumberOfFrames;
    int16_t Unknown1;//apparently always 1
    int32_t Unknown2;//apprently always 0
    char Name[32];
};

struct FILE_GafFrameEntry
{
    int32_t FrameInfoOffset;
    int32_t Unknown1;//completely unknown
};

struct FILE_GafFrameData
{
    int16_t Width;
    int16_t Height;
    int16_t XPos;
    int16_t YPos;
    char Unknown1;//apparently always 9?
    char Compressed;
    int16_t NumberOfSubframes;
    int32_t Unknown2;//always 0
    int32_t FrameDataOffset;
    int32_t Unknow3;//completely unknown
};

struct FILE_PCXHeader
{
    uint8_t Manufacturer;//should always be 0x0A
    uint8_t Version;
    uint8_t Encoding;
    uint8_t BitsPerPlane;
    uint16_t XMin, YMin, XMax, YMax, VerticalDPIOrResolution, HorizontalDPIOrResolution /* this can apparently be ommitted??? */;
    uint8_t palette[48], reserved, ColorPlanes;
    uint16_t BytesPerPlaneLine, PalletteType, HorizontalScrSize, VerticalScrSize;
    uint8_t Pad[54];
};
#pragma pack(pop)


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

void SetupTextureContainer(TextureContainer * TextureContainer,int Width, int Height, int MaxTextures, MemoryArena * Arena)
{
    *TextureContainer = {};
    TextureContainer->MaximumTextures = MaxTextures;
    TextureContainer->TextureWidth = Width;
    TextureContainer->TextureHeight = Height;
    TextureContainer->TextureData = PushArray(Arena, Width*Height*4, uint8_t);
    TextureContainer->Textures = PushArray(Arena, MaxTextures, Texture);

    int size=TextureContainer->TextureWidth * TextureContainer->TextureHeight*4/4;
  
    int32_t * ClearDataPointer=(int32_t *)TextureContainer->TextureData;
    for(int i=0;i<size;i++)
	ClearDataPointer[i]=0xdeadbeef;
}



Texture * GetTexture(const char * Name,TextureContainer * TextureContainer)
{
    //TODO(Christof): some better storage/retrieval mechanism for texture lookup?
    for(int i=0;i<TextureContainer->NumberOfTextures;i++)
    {
	if(CaseInsensitiveMatch(Name,TextureContainer->Textures[i].Name))
	    return &TextureContainer->Textures[i];
    }
    return 0;
}

inline bool32 TexturePixelFree(int X, int Y,TextureContainer * TextureContainer)
{
    return *(int32_t *)&TextureContainer->TextureData[(X+Y*TextureContainer->TextureWidth)*4]==0xdeadbeef;
}

TexturePosition GetAvailableTextureLocation(int Width, int Height, TextureContainer * TextureContainer)
{
    int StartX=TextureContainer->FirstFreeTexture.X;
    for(int Y=TextureContainer->FirstFreeTexture.Y;Y<TextureContainer->TextureHeight;Y++)
    {
	for(int X=StartX;X<TextureContainer->TextureWidth;X++)
	{
	    if(Y+Height>=TextureContainer->TextureHeight || X+ Width >= TextureContainer->TextureWidth)
		continue;
	    //TODO(Christof): figure out a better way to do this without goto
	    for(int TexY=0;TexY<Height;TexY++)
	    {
		for(int TexX=0;TexX<Width;TexX++)
		{
		    if(!TexturePixelFree(X+TexX,Y+TexY,TextureContainer))
		    {
			goto next;
		    }
		}
	    }
	    //NOTE: if texure search has to loop (total textures size -> big texture size) this will get quite slow, should be ok for now though
	    TextureContainer->FirstFreeTexture.Y=Y;
	    TextureContainer->FirstFreeTexture.X=X;
	    return {X,Y};
	next:
	    continue;
	}
	StartX=0;
    }
    return {-1,-1};
}

void LoadGafTextureData(uint8_t * Buffer, FILE_GafFrameData * Frame, TexturePosition OriginalPosition, TexturePosition PositionToStore, TextureContainer * TextureContainer, uint8_t * PaletteData)
{
    uint8_t * TextureData = TextureContainer->TextureData;
    uint8_t * FrameData= (uint8_t*)(Buffer + Frame->FrameDataOffset);
    if(Frame->Compressed)
    {
	for(int y=0;y<Frame->Height;y++)
	{
	    int LineBytes = *(int16_t*)FrameData;
	    FrameData +=2;
	    int x=0;
	    while(LineBytes>0)
	    {
		uint8_t mask = *FrameData++;
		LineBytes--;
		if(mask & 0x01)
		{
		    int count = mask >>1;
		    x+=count;
		}
		else if(mask & 0x02)
		{
		    int count = (mask >>2) +1;
		    int ColorIndex = *FrameData++;
		    LineBytes--;
		    for(;count>0;count--)
		    {
			if(x+PositionToStore.X>= OriginalPosition.X && y+PositionToStore.Y >= OriginalPosition.Y)
			{
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+0]=PaletteData[ColorIndex*4+0];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+1]=PaletteData[ColorIndex*4+1];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+2]=PaletteData[ColorIndex*4+2];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+3]=255;
			}
			x++;
		    }
			
		}
		else
		{
		    int count = (mask >>2 )+1;
		    for(;count>0;count--)
		    {
			if(x+PositionToStore.X>= OriginalPosition.X && y+PositionToStore.Y >= OriginalPosition.Y)
			{
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+0]=PaletteData[*FrameData*4+0];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+1]=PaletteData[*FrameData*4+1];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+2]=PaletteData[*FrameData*4+2];
			    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+3]=255;
			}
			x++;
			FrameData++;
			LineBytes--;
		    }
		}
	    }
	}
    }
    else
    {
	for(int y=0;y<Frame->Height;y++)
	{
	    for(int x=0;x<Frame->Width;x++)
	    {
		if(x+PositionToStore.X>= OriginalPosition.X && y+PositionToStore.Y >= OriginalPosition.Y)
		{
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+0]=PaletteData[FrameData[x+y*Frame->Width]*4+0];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+1]=PaletteData[FrameData[x+y*Frame->Width]*4+1];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+2]=PaletteData[FrameData[x+y*Frame->Width]*4+2];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TextureContainer->TextureWidth)*4+3]=255;
		}
	    }
	}
	//NOTE: Move this out if we ever start handling Compressed textures
    }

}

void LoadGafFrameEntry(uint8_t * Buffer, int Offset, TextureContainer * TextureContainer, uint8_t * PaletteData)
{
    FILE_GafEntry * Entry = (FILE_GafEntry*)(Buffer +Offset);
    FILE_GafFrameEntry * FrameEntries=(FILE_GafFrameEntry *)(Buffer + Offset + sizeof(*Entry));
    Texture * Textures = TextureContainer->Textures;
    //NOTE(Christof): some Gafs have frames whose dimensions (height in original TA textures) do not match frame 0
    Texture * Texture=GetTexture(Entry->Name,TextureContainer);
    if(Texture)
    {
	LogInformation("Skipping %s as it has already been loaded",Entry->Name);
	return;
    }
    int NextTexture = TextureContainer->NumberOfTextures;
    int TotalWidth =0;
    int MaxHeight =0;
    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	TotalWidth += Frame->Width;
	if(MaxHeight < Frame->Height)
	    MaxHeight = Frame->Height;
    }
    TexturePosition PositionToStore = GetAvailableTextureLocation(TotalWidth,MaxHeight, TextureContainer);
    if(PositionToStore.X==-1)
    {
	LogError("Unable to get storage location for texture: %s (%dx%d)",Entry->Name,TotalWidth,MaxHeight);
	return;
    }
    for(int y=0;y<MaxHeight;y++)
    {
	memset(TextureContainer->TextureData + ((y+PositionToStore.Y)*TextureContainer->TextureWidth *4) +PositionToStore.X*4, 0, TotalWidth *4); 
    }
    Textures[NextTexture].U=PositionToStore.X/float(TextureContainer->TextureWidth);
    Textures[NextTexture].V=PositionToStore.Y/float(TextureContainer->TextureHeight);
    Textures[NextTexture].NumberOfFrames = Entry->NumberOfFrames;
    memcpy(Textures[NextTexture].Name,Entry->Name,32);

    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	if(Frame->NumberOfSubframes!=0)
	{
	    int32_t * SubFrameOffsets = (int32_t *)(Buffer + Frame->FrameDataOffset);
	    for(int x=0;x<Frame->NumberOfSubframes;x++)
	    {
		FILE_GafFrameData * SubFrame = (FILE_GafFrameData*)(Buffer + SubFrameOffsets[x]);
		TexturePosition currentpos = {PositionToStore.X + (Frame->XPos - SubFrame->XPos), PositionToStore.Y +(Frame->YPos - SubFrame->YPos)};
		LoadGafTextureData(Buffer, SubFrame, PositionToStore, currentpos, TextureContainer, PaletteData);
	    }
	}
	else
	{
	    //NOTE: Many textures have non-zero X and/or Y, perhaps this influences texture coord gen in some way? rendering in the past has ignored these values and seemed fine, so going to just ignore them for now
	    LoadGafTextureData(Buffer, Frame, PositionToStore, PositionToStore, TextureContainer, PaletteData);
	}
	Textures[NextTexture].Widths[i]=Frame->Width/float(TextureContainer->TextureWidth);
	Textures[NextTexture].Heights[i]=Frame->Height/float(TextureContainer->TextureHeight);
	PositionToStore.X += Frame->Width;
	Textures[NextTexture].X[i]=Frame->XPos;
	Textures[NextTexture].Y[i]=Frame->YPos;
    }
    TextureContainer->NumberOfTextures++;
}

void LoadTexturesFromGafBuffer(uint8_t * Buffer,TextureContainer * TextureContainer, uint8_t * PalletteData)
{
    FILE_GafHeader * header= (FILE_GafHeader *)Buffer;
    if(header->IDVersion != GAF_IDVERSION)
    {
	LogError("Unknown Gaf IDVersion detected: %d",header->IDVersion);
	return;
    }
    int32_t * EntryOffsets = (int32_t *)(Buffer + sizeof(*header));
    for(int i=0;i<header->NumberOfEntries;i++)
    {
	LoadGafFrameEntry(Buffer, EntryOffsets[i], TextureContainer, PalletteData);
    }
}


#include <math.h>
void LoadPalette(GameState * CurrentGameState)
{

    HPIEntry Palette = FindEntryInAllFiles("palettes/PALETTE.PAL",CurrentGameState);
    if(!Palette.Name)
    {
	LogWarning("No Palette found in data files");
    }
    else if(Palette.IsDirectory)
    {
	LogWarning("Directory found while trying to load palette");
    }
    else
    {
	CurrentGameState->PaletteLoaded=1;
	LoadHPIFileEntryData(Palette,CurrentGameState->PaletteData,&CurrentGameState->TempArena);
    }
}

void LoadAllTexturesFromHPIEntry(HPIEntry * Textures, TextureContainer * TextureContainer, MemoryArena * TempArena,uint8_t * PaletteData)
{
    if(!Textures->IsDirectory)
    {
	uint8_t * GafBuffer = PushArray(TempArena, Textures->File.FileSize,uint8_t);
	LoadHPIFileEntryData(*Textures,GafBuffer,TempArena);
	LoadTexturesFromGafBuffer(GafBuffer,TextureContainer,PaletteData);
	PopArray(TempArena, GafBuffer,  Textures->File.FileSize, uint8_t);
    }
    else
    {
	for(int i=0;i<Textures->Directory.NumberOfEntries;i++)
	{
	    if(Textures->Directory.Entries[i].IsDirectory)
	    {
		LogWarning("Unexpectedly found directory %s inside textures directory of %s",Textures->Directory.Entries[i].Name, Textures->Directory.Entries[i].ContainedInFile->Name);
	    }
	    //uint8_t GafBuffer[Textures->Directory.Entries[i].File.FileSize];
	    //STACK_ARRAY(GafBuffer,Textures->Directory.Entries[i].File.FileSize,uint8_t);
	    uint8_t * GafBuffer = PushArray(TempArena, Textures->Directory.Entries[i].File.FileSize,uint8_t);
	    LoadHPIFileEntryData(Textures->Directory.Entries[i],GafBuffer,TempArena);
	    LoadTexturesFromGafBuffer(GafBuffer,TextureContainer, PaletteData);
	    PopArray(TempArena, GafBuffer,Textures->Directory.Entries[i].File.FileSize,uint8_t);
	}    
    }

    glGenTextures(1,&TextureContainer->Texture);
    glBindTexture(GL_TEXTURE_2D,TextureContainer->Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TextureContainer->TextureWidth,TextureContainer->TextureHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureContainer->TextureData);
}

void LoadAllTextures(GameState * CurrentGameState)
{
    if(!CurrentGameState->PaletteLoaded)
    {
	LoadPalette(CurrentGameState);
    }   
    
    HPIEntry Textures = FindEntryInAllFiles("textures",CurrentGameState);
    if(!Textures.Name)
    {
	LogWarning("No Textures found in archives");
	return;
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&Textures, CurrentGameState->UnitTextures, &CurrentGameState->TempArena, CurrentGameState->PaletteData);
    }

    UnloadCompositeEntry(&Textures,&CurrentGameState->TempArena);
}




Texture * AddPCXToTextureContainer(TextureContainer * Textures, const char * FileName, GameState * CurrentGameState)
{
    HPIEntry PCX = FindEntryInAllFiles(FileName, CurrentGameState);
    if(PCX.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load %s", FileName);
    }
    else
    {
	uint8_t * PCXBuffer = PushArray(&CurrentGameState->TempArena, PCX.File.FileSize,uint8_t);
	LoadHPIFileEntryData(PCX,PCXBuffer,&CurrentGameState->TempArena);

	FILE_PCXHeader * Header = (FILE_PCXHeader*)PCXBuffer;
	int Width = Header->XMax - Header->XMin +1;
	int Height = Header->YMax - Header->YMin +1;
	if(Header->BitsPerPlane != 8 || Header->Encoding != 1 || Header->ColorPlanes != 1 || Header->PalletteType != 1 || Header->BytesPerPlaneLine != Width || Header->YMin != 0 || Header->XMin !=0 || Header->Version != 5)
	{
	    LogError("Unsupported PCX %s not loaded.", FileName);
	    PopArray(&CurrentGameState->TempArena, PCXBuffer,  PCX.File.FileSize, uint8_t);
	    return 0;
	}

	uint8_t * PaletteData = PCXBuffer + (PCX.File.FileSize - (256*3));
	if(*(PaletteData -1)!=0x0C)
	{
	    LogError("Could not find expected palette in PCX %s", FileName);
	    PopArray(&CurrentGameState->TempArena, PCXBuffer,  PCX.File.FileSize, uint8_t);
	    return 0;
	}

	TexturePosition PCXPos = GetAvailableTextureLocation(Width, Height, Textures);

	if(PCXPos.X == -1)
	{
	    LogError("Could not fit %s into texture (%dx%d)",FileName,Width,Height);
	    PopArray(&CurrentGameState->TempArena, PCXBuffer,  PCX.File.FileSize, uint8_t);
	    return 0;
	}

	int X=0, Y=0;
	uint8_t * ImageBuffer = PCXBuffer + 0x80;
	uint8_t * TextureData = PushArray(&CurrentGameState->TempArena, Width*Height*4, uint8_t) ;

	//TODO(Christof): Transparency, might be default TA blue, or the horrible FF00FF pink
	while(ImageBuffer < PaletteData && Y<Height)
	{
	    uint8_t ColorOrCount = *ImageBuffer++;
	    if((ColorOrCount & 0xC0) == 0xC0)
	    {
		int count = ColorOrCount & 0x3F;
		int ColorIndex = *ImageBuffer++;
		while(count-->0)
		{
		   
			TextureData[(X+Y*Width)*4+0]=PaletteData[ColorIndex*3+0];
			TextureData[(X+Y*Width)*4+1]=PaletteData[ColorIndex*3+1];
			TextureData[(X+Y*Width)*4+2]=PaletteData[ColorIndex*3+2];
			TextureData[(X+Y*Width)*4+3]=255;
		  
		    X++;
		}
	    }
	    else
	    {
		int ColorIndex = ColorOrCount;
	
		    TextureData[(X+Y*Width)*4+0]=PaletteData[ColorIndex*3+0];
		    TextureData[(X+Y*Width)*4+1]=PaletteData[ColorIndex*3+1];
		    TextureData[(X+Y*Width)*4+2]=PaletteData[ColorIndex*3+2];
		    TextureData[(X+Y*Width)*4+3]=255;
		
		
		X++;
	    }
	    if(X>=Width)
	    {
		X=0;
		Y++;
	    }
	}

	glBindTexture(GL_TEXTURE_2D, Textures->Texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, PCXPos.X, PCXPos.Y, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	
	PopArray(&CurrentGameState->TempArena, TextureData,  Width*Height*4, uint8_t);
	PopArray(&CurrentGameState->TempArena, PCXBuffer,  PCX.File.FileSize, uint8_t);

	int NextTexture = Textures->NumberOfTextures;
	Textures->Textures[NextTexture].Widths[0]=Width/float( Textures->TextureWidth);
	Textures->Textures[NextTexture].Heights[0]=Height/float( Textures->TextureHeight);
	Textures->Textures[NextTexture].X[0]=0;
	Textures->Textures[NextTexture].Y[0]=0;
	Textures->Textures[NextTexture].U=PCXPos.X/float(Textures->TextureWidth);
	Textures->Textures[NextTexture].V=PCXPos.Y/float(Textures->TextureHeight);
	Textures->Textures[NextTexture].NumberOfFrames = 1;
	
	
	return &Textures->Textures[Textures->NumberOfTextures++];
    }

    return 0;
}
