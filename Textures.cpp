

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
#pragma pack(pop)


struct TexturePosition
{
    int X;
    int Y;
};

#define MAX_NUMBER_OF_TEXTURE_FRAMES 12

struct Texture
{
    char Name[32];
    int NumberOfFrames;
    float U,V;
    float Widths[MAX_NUMBER_OF_TEXTURE_FRAMES],Heights[MAX_NUMBER_OF_TEXTURE_FRAMES];
};



TexturePosition GetAvailableTextureLocation(int Width, int Height,uint8_t * TextureData);
void SetTextureLocationUsed(int X, int Y, int Width, int Height);

Texture * GetTexture(const char * Name,int NextTexture,Texture * Textures)
{
    //TODO(Christof): some better storage/retrieval mechanism for texture lookup?
    for(int i=0;i<NextTexture;i++)
    {
	if(CaseInsensitiveMatch(Name,Textures[i].Name))
	    return &Textures[i];
    }
    return 0;
}

void LoadGafFrameEntry(uint8_t * Buffer, int Offset,GameState * CurrentGameState)
{
    FILE_GafEntry * Entry = (FILE_GafEntry*)(Buffer +Offset);
    FILE_GafFrameEntry * FrameEntries=(FILE_GafFrameEntry *)(Buffer + Offset + sizeof(*Entry));
    Texture * Textures = CurrentGameState->Textures;
    uint8_t * TextureData = CurrentGameState->TextureData;
    uint8_t * PaletteData = CurrentGameState->PaletteData;
    //NOTE(Christof): some Gafs have frames whose dimensions (height in original TA textures) do not match frame 0
    Texture * Texture=GetTexture(Entry->Name,CurrentGameState->NextTexture,CurrentGameState->Textures);
    if(Texture)
    {
	LogInformation("Skipping %s as it has already been loaded");
	return;
    }
    int NextTexture = CurrentGameState->NextTexture;
    int TotalWidth =0;
    int MaxHeight =0;
    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	if(Frame->Height > MaxHeight)
	    MaxHeight = Frame->Height;
	TotalWidth += Frame->Width;
    }
    TexturePosition PositionToStore = GetAvailableTextureLocation(TotalWidth,MaxHeight, CurrentGameState->TextureData);
    if(PositionToStore.X==-1)
    {
	LogError("Unable to get storage location for texture: %s",Entry->Name);
	return;
    }
    Textures[NextTexture].U=PositionToStore.X/float(TEXTURE_WIDTH);
    Textures[NextTexture].V=PositionToStore.Y/float(TEXTURE_HEIGHT);
    Textures[NextTexture].NumberOfFrames = Entry->NumberOfFrames;
    memcpy(Textures[NextTexture].Name,Entry->Name,32);

    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	if(Frame->NumberOfSubframes!=0)
	{
	    //NOTE: no textures from totala1.hpi have subframes, ignoring for now
	    LogError("%s(%d) has %d subframes",Entry->Name,i,Frame->NumberOfSubframes);
	    continue;
	}
	
	//NOTE: Many textures have non-zero X and/or Y, perhaps this influences texture coord gen in some way? rendering in the past has ignored these values and seemed fine, so going to just ignore them for now

	uint8_t * FrameData= (uint8_t*)(Buffer + Frame->FrameDataOffset);
	if(Frame->Compressed)
	{
	    LogWarning("Compressed Data not currently supported for %s(%d)",Entry->Name,i);
	}
	else
	{
	    for(int y=0;y<Frame->Height;y++)
	    {
		for(int x=0;x<Frame->Width;x++)
		{
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TEXTURE_WIDTH)*4+0]=PaletteData[FrameData[x+y*Frame->Width]*4+0];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TEXTURE_WIDTH)*4+1]=PaletteData[FrameData[x+y*Frame->Width]*4+1];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TEXTURE_WIDTH)*4+2]=PaletteData[FrameData[x+y*Frame->Width]*4+2];
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TEXTURE_WIDTH)*4+3]=255;
		}
	    }
	    //NOTE: Move this out if we ever start handling Compressed textures
	}
	Textures[NextTexture].Widths[i]=Frame->Width/float(TEXTURE_WIDTH);
	Textures[NextTexture].Heights[i]=Frame->Height/float(TEXTURE_HEIGHT);
	PositionToStore.X += Frame->Width;
    }
    CurrentGameState->NextTexture++;
}

void LoadTexturesFromGafBuffer(uint8_t * Buffer,GameState * CurrentGameState)
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
	LoadGafFrameEntry(Buffer, EntryOffsets[i], CurrentGameState);
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


void LoadTextures(HPIFile* HPI)
{
   
}




bool32 LoadAllTextures(GameState * CurrentGameState)
{
    //Clear Texture data so we can lookup whether a space is free
    int size=TEXTURE_WIDTH *TEXTURE_HEIGHT*4/4;
    int32_t * ClearDataPointer=(int32_t *)CurrentGameState->TextureData;
    for(int i=0;i<size;i++)
	ClearDataPointer[i]=0xdeadbeef;

    if(!CurrentGameState->PaletteLoaded)
    {
	LoadPalette(CurrentGameState);
    }
    HPIEntry Textures = FindEntryInAllFiles("textures",CurrentGameState);
    if(!Textures.Name)
    {
	LogWarning("No Textures found in archives");
	return 0;
    }
    if(!Textures.IsDirectory)
    {
	LogError("Found a file instead of a directory while trying to load textures from archives");
	return 0;
    }
    for(int i=0;i<Textures.Directory.NumberOfEntries;i++)
    {
	if(Textures.Directory.Entries[i].IsDirectory)
	{
	    LogWarning("Unexpectedly found directory %s inside textures directory of %s",Textures.Directory.Entries[i].Name,
		       Textures.Directory.Entries[i].ContainedInFile->Name);
	}
	//uint8_t GafBuffer[Textures.Directory.Entries[i].File.FileSize];
	//STACK_ARRAY(GafBuffer,Textures.Directory.Entries[i].File.FileSize,uint8_t);
	uint8_t * GafBuffer = PushArray(&CurrentGameState->TempArena, Textures.Directory.Entries[i].File.FileSize,uint8_t);
	LoadHPIFileEntryData(Textures.Directory.Entries[i],GafBuffer,&CurrentGameState->TempArena);
	LoadTexturesFromGafBuffer(GafBuffer,CurrentGameState);
	PopArray(&CurrentGameState->TempArena, GafBuffer,  Textures.Directory.Entries[i].File.FileSize,uint8_t);
    }
    UnloadCompositeEntry(&Textures,&CurrentGameState->TempArena);
    
    glGenTextures(1,&CurrentGameState->UnitTexture);
    glBindTexture(GL_TEXTURE_2D,CurrentGameState->UnitTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TEXTURE_WIDTH,TEXTURE_HEIGHT,0, GL_RGBA, GL_UNSIGNED_BYTE, CurrentGameState->TextureData);
    return 1;

}

inline bool32 TexturePixelFree(int X, int Y,uint8_t * TextureData)
{
    return *(int32_t *)&TextureData[(X+Y*TEXTURE_WIDTH)*4]==0xdeadbeef;
}



TexturePosition GetAvailableTextureLocation(int Width, int Height, uint8_t *TextureData)
{
    static TexturePosition FirstFreeTexture = {};
    int StartX=FirstFreeTexture.X;
    for(int Y=FirstFreeTexture.Y;Y<TEXTURE_HEIGHT;Y++)
    {
	for(int X=StartX;X<TEXTURE_WIDTH;X++)
	{
	    if(Y+Height>=TEXTURE_HEIGHT || X+ Width >= TEXTURE_WIDTH)
		continue;
	    //TODO(Christof): figure out a better way to do this without goto
	    for(int TexY=0;TexY<Height;TexY++)
	    {
		for(int TexX=0;TexX<Width;TexX++)
		{
		    if(!TexturePixelFree(X+TexX,Y+TexY,TextureData))
		    {
			goto next;
		    }
		}
	    }
	    //NOTE: if texure search has to loop (total textures size -> big texture size) this will get quite slow, should be ok for now though
	    FirstFreeTexture.X=X;
	    FirstFreeTexture.Y=Y;
	    return {X,Y};
	next:
	    continue;
	}
	StartX=0;
    }
    return {-1,-1};
}

