#define Align(Val, Align) (Val +((Align) -1) & ~((Align)-1))

void SetupTextureContainer(TextureContainer * TextureContainer,int Width, int Height, int MaxTextures, MemoryArena * Arena)
{
    *TextureContainer = {};
    TextureContainer->MaximumTextures = MaxTextures;
    TextureContainer->TextureWidth = Align(Width, PIXELS_PER_SQUARE_SIDE);
    TextureContainer->TextureHeight = Align(Height, PIXELS_PER_SQUARE_SIDE);
    TextureContainer->TextureData = PushArray(Arena, TextureContainer->TextureWidth*TextureContainer->TextureHeight*4, u8 );
    TextureContainer->Textures = PushArray(Arena, MaxTextures, Texture);
    TextureContainer->WidthInSquares = Align(Width,PIXELS_PER_SQUARE_SIDE) / PIXELS_PER_SQUARE_SIDE;
    TextureContainer->HeightInSquares = Align(Height, PIXELS_PER_SQUARE_SIDE) / PIXELS_PER_SQUARE_SIDE;
    int FreeSquareSize = TextureContainer->HeightInSquares * TextureContainer->WidthInSquares / 8;
    TextureContainer->FreeSquares = PushArray(Arena, FreeSquareSize , u8 );
    memset(TextureContainer->FreeSquares, 0, FreeSquareSize);
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

 b32 SquareIsFree(TextureContainer * TextureContainer, int x, int y)
{
     int Offset = y*TextureContainer->WidthInSquares + x;
     int Index = Offset / 8;
     int BitOffset = Offset % 8;
     return !((TextureContainer->FreeSquares[Index]>>BitOffset)&1);
}

TexturePosition GetAvailableTextureLocation(int Width, int Height, TextureContainer * TextureContainer)
{
    int WidthInSquares = Align(Width,PIXELS_PER_SQUARE_SIDE) / PIXELS_PER_SQUARE_SIDE;
    int HeightInSquares = Align(Height,PIXELS_PER_SQUARE_SIDE) / PIXELS_PER_SQUARE_SIDE;

    for(int Y=0;Y<TextureContainer->HeightInSquares;Y++)
    {
	for(int X=0;X<TextureContainer->WidthInSquares;X++)
	{
	    if(Y+HeightInSquares>TextureContainer->HeightInSquares || X+ WidthInSquares > TextureContainer->WidthInSquares)
		continue;

		    
	    for(int TexY=0;TexY<HeightInSquares;TexY++)
	    {
		for(int TexX=0;TexX<WidthInSquares;TexX++)
		{
		    if(!SquareIsFree(TextureContainer, X+TexX, Y+TexY))
		    {
			goto next;
		    }
		}
	    }

	    for(int TexY=0;TexY<HeightInSquares;TexY++)
	    {
		for(int TexX=0;TexX<WidthInSquares;TexX++)
		{
		    int Offset = (Y+TexY)*TextureContainer->WidthInSquares + (X+TexX);
		    int Index = Offset / 8;
		    int BitOffset = Offset % 8;
		    TextureContainer->FreeSquares[Index] |= 1<<BitOffset;
		}
	    }
	    return {X*PIXELS_PER_SQUARE_SIDE,Y*PIXELS_PER_SQUARE_SIDE};
	next:
	    continue;
	}
    }
    return {-1,-1};
}

void LoadGafTextureData(u8 * Buffer, FILE_GafFrameData * Frame, TexturePosition OriginalPosition, TexturePosition PositionToStore, TextureContainer * TextureContainer, u8 * PaletteData)
{
    u8 * TextureData = TextureContainer->TextureData;
    u8 * FrameData= (u8 *)(Buffer + Frame->FrameDataOffset);
    if(Frame->Compressed)
    {
	for(int y=0;y<Frame->Height;y++)
	{
	    int LineBytes = *(s16 *)FrameData;
	    FrameData +=2;
	    int x=0;
	    while(LineBytes>0)
	    {
		u8 mask = *FrameData++;
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

void LoadGafFrameEntry(u8 * Buffer, int Offset, TextureContainer * TextureContainer, u8 * PaletteData)
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
	LogError("Unable to get storage location for texture: %s (%dx%d), Texture: %dx%d",Entry->Name,TotalWidth,MaxHeight, TextureContainer->TextureWidth, TextureContainer->TextureHeight);
	return;
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
	    s32 * SubFrameOffsets = (s32  *)(Buffer + Frame->FrameDataOffset);
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

void LoadTexturesFromGafBuffer(u8 * Buffer,TextureContainer * TextureContainer, u8 * PalletteData)
{
    FILE_GafHeader * header= (FILE_GafHeader *)Buffer;
    if(header->IDVersion != GAF_IDVERSION)
    {
	LogError("Unknown Gaf IDVersion detected: %d",header->IDVersion);
	return;
    }
    s32 * EntryOffsets = (s32  *)(Buffer + sizeof(*header));
    for(int i=0;i<header->NumberOfEntries;i++)
    {
	LoadGafFrameEntry(Buffer, EntryOffsets[i], TextureContainer, PalletteData);
    }
}


#include <math.h>
void LoadPalette(GameState * CurrentGameState)
{

    HPIEntry Palette = FindEntryInAllFiles("palettes/PALETTE.PAL",&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
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
	LoadHPIFileEntryData(Palette,CurrentGameState->PaletteData,&CurrentGameState->TempArena);
    }
}

void LoadAllTexturesFromHPIEntry(HPIEntry * Textures, TextureContainer * TextureContainer, MemoryArena * TempArena,u8 * PaletteData)
{
    if(!Textures->IsDirectory)
    {
	u8 * GafBuffer = PushArray(TempArena, Textures->File.FileSize,u8 );
	LoadHPIFileEntryData(*Textures,GafBuffer,TempArena);
	LoadTexturesFromGafBuffer(GafBuffer,TextureContainer,PaletteData);
	PopArray(TempArena, GafBuffer,  Textures->File.FileSize, u8 );
    }
    else
    {
	for(int i=0;i<Textures->Directory.NumberOfEntries;i++)
	{
	    if(Textures->Directory.Entries[i].IsDirectory)
	    {
		LogWarning("Unexpectedly found directory %s inside textures directory of %s",Textures->Directory.Entries[i].Name, Textures->Directory.Entries[i].ContainedInFile->Name);
	    }
	    //u8 GafBuffer[Textures->Directory.Entries[i].File.FileSize];
	    //STACK_ARRAY(GafBuffer,Textures->Directory.Entries[i].File.FileSize,u8 );
	    u8 * GafBuffer = PushArray(TempArena, Textures->Directory.Entries[i].File.FileSize,u8 );
	    LoadHPIFileEntryData(Textures->Directory.Entries[i],GafBuffer,TempArena);
	    LoadTexturesFromGafBuffer(GafBuffer,TextureContainer, PaletteData);
	    PopArray(TempArena, GafBuffer,Textures->Directory.Entries[i].File.FileSize,u8 );
	}    
    }

    glGenTextures(1,&TextureContainer->Texture);
    glBindTexture(GL_TEXTURE_2D,TextureContainer->Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TextureContainer->TextureWidth,TextureContainer->TextureHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureContainer->TextureData);
}

void LoadAllUnitTextures(HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena, TextureContainer * UnitTextures, u8 * PaletteData)
{
    HPIEntry Textures = FindEntryInAllFiles("textures",GlobalArchiveCollection, TempArena);
    if(!Textures.Name)
    {
	LogWarning("No Textures found in archives");
	return;
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&Textures, UnitTextures, TempArena, PaletteData);
    }

    UnloadCompositeEntry(&Textures,TempArena);
}

Texture * AddPCXToTextureContainer(TextureContainer * Textures, const char * FileName, HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena)
{
    HPIEntry PCX = FindEntryInAllFiles(FileName, GlobalArchiveCollection, TempArena);
    if(PCX.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load %s", FileName);
    }
    else
    {
	u8 * PCXBuffer = PushArray(TempArena, PCX.File.FileSize,u8 );
	LoadHPIFileEntryData(PCX,PCXBuffer,TempArena);

	FILE_PCXHeader * Header = (FILE_PCXHeader*)PCXBuffer;
	int Width = Header->XMax - Header->XMin +1;
	int Height = Header->YMax - Header->YMin +1;
	if(Header->BitsPerPlane != 8 || Header->Encoding != 1 || Header->ColorPlanes != 1 || Header->PalletteType != 1 || Header->BytesPerPlaneLine != Width || Header->YMin != 0 || Header->XMin !=0 || Header->Version != 5)
	{
	    LogError("Unsupported PCX %s not loaded.", FileName);
	    PopArray(TempArena, PCXBuffer,  PCX.File.FileSize, u8 );
	    return 0;
	}

	u8 * PaletteData = PCXBuffer + (PCX.File.FileSize - (256*3));
	if(*(PaletteData -1)!=0x0C)
	{
	    LogError("Could not find expected palette in PCX %s", FileName);
	    PopArray(TempArena, PCXBuffer,  PCX.File.FileSize, u8 );
	    return 0;
	}

	TexturePosition PCXPos = GetAvailableTextureLocation(Width, Height, Textures);

	if(PCXPos.X == -1)
	{
	    LogError("Could not fit %s into texture (%dx%d)",FileName,Width,Height);
	    PopArray(TempArena, PCXBuffer,  PCX.File.FileSize, u8 );
	    return 0;
	}

	int X=0, Y=0;
	u8 * ImageBuffer = PCXBuffer + 0x80;
	u8 * TextureData = PushArray(TempArena, Width*Height*4, u8 ) ;

	//TODO(Christof): Transparency, might be default TA blue, or the horrible FF00FF pink
	while(ImageBuffer < PaletteData && Y<Height)
	{
	    u8 ColorOrCount = *ImageBuffer++;
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
	
	PopArray(TempArena, TextureData,  Width*Height*4, u8 );
	PopArray(TempArena, PCXBuffer,  PCX.File.FileSize, u8 );

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
