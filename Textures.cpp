
//NOTE: Textures in "textures"


const int32_t GAF_IDVERSION=0x00010100;
struct FILE_GafHeader
{
    int32_t IDVersion;
    int32_t NumberOfEntries;
    int32_t Unknown1; //Always 0 apparently?
};

struct __attribute__((__packed__)) FILE_GafEntry
{
    int16_t NumberOfFrames;
    int16_t Unknown1;//apparently always 1
    int32_t Unknown2;//apprently always 0
    char Name[32];
};

struct __attribute__((__packed__)) FILE_GafFrameEntry
{
    int32_t FrameInfoOffset;
    int32_t Unknown1;//completely unknown
};

struct __attribute__((__packed__)) FILE_GafFrameData
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

struct TexturePosition
{
    int X;
    int Y;
};

struct Texture
{
    char Name[32];
    int FrameNumber;
    float U,V;
    float Width,Height;
};

const int TEXTURE_WIDTH=2048;
const int TEXTURE_HEIGHT=2048;
char TextureData[TEXTURE_HEIGHT*TEXTURE_WIDTH*4];

char PaletteData[1024];
bool32 PaletteLoaded=0;

TexturePosition GetAvailableTextureLocation(int Width, int Height);
void SetTextureLocationUsed(int X, int Y, int Width, int Height);

void LoadGafFrameEntry(char * Buffer, int Offset)
{
    FILE_GafEntry * Entry = (FILE_GafEntry*)(Buffer +Offset);
    FILE_GafFrameEntry * FrameEntries=(FILE_GafFrameEntry *)(Buffer + Offset + sizeof(*Entry));

    //TODO(Christof): Actually load frame data here
    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	int Width=Frame->Width, Height = Frame->Height;
	if(Frame->NumberOfSubframes!=0)
	{
	    //NOTE: no textures from totala1.hpi have subframes, ignoring for now
	    LogError("%s(%d) has %d subframes",Entry->Name,i,Frame->NumberOfSubframes);
	    continue;
	}
		
	
	//NOTE: Many textures have non-zero X and/or Y, perhaps this influences texture coord gen in some way? rendering in the past has ignored these values and seemed fine, so going to just ignore them for now


	TexturePosition PositionToStore = GetAvailableTextureLocation(Width,Height);
	if(PositionToStore.X==-1)
	{
	    LogError("Unable to get storage location for texture: %s(%d of %d)",Entry->Name,i,Entry->NumberOfFrames);
	    continue;
	}
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
		    TextureData[(x+PositionToStore.X+(y+PositionToStore.Y)*TEXTURE_WIDTH)*4+3]=255;//PaletteData[FrameData[x+y*Frame->Width]*4+3];
		}
	    }
	}
    }
}

void LoadTexturesFromGafBuffer(char * Buffer)
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
	LoadGafFrameEntry(Buffer, EntryOffsets[i]);
    }
}

#include <math.h>

void LoadTextures(HPIFile* HPI)
{
    if(!PaletteLoaded)
    {
	
	HPIEntry Palette = FindHPIEntry(HPI,"palettes/PALETTE.PAL");
	if(!Palette.Name)
	{
	    LogWarning("No Palette found in %s",HPI->Name);
	}
	else if(Palette.IsDirectory)
	{
	    LogWarning("Directory found while trying to load palette");
	}
	else
	{
	    PaletteLoaded=1;
	    LoadHPIFileEntryData(Palette,PaletteData);
	}
    }
    HPIEntry Textures = FindHPIEntry(HPI,"textures");
    if(!Textures.Name)
    {
	LogWarning("No Textures found in %s",HPI->Name);
	return;
    }
    if(!Textures.IsDirectory)
    {
	LogError("Found a file instead of a directory while trying to load textures from %s",HPI->Name);
	return;
    }
    for(int i=0;i<Textures.Directory.NumberOfEntries;i++)
    {
	if(Textures.Directory.Entries[i].IsDirectory)
	{
	    LogWarning("Unexpectedly found directory %s inside textures directory of %s",Textures.Directory.Entries[i].Name,HPI->Name);
	}
	char GafBuffer[Textures.Directory.Entries[i].File.FileSize];
	LoadHPIFileEntryData(Textures.Directory.Entries[i],GafBuffer);
	LoadTexturesFromGafBuffer(GafBuffer);
    }
    TexturePosition FirstFree = GetAvailableTextureLocation(128,128);
    LogDebug("First Free Texture Position: (%d,%d)",FirstFree.X,FirstFree.Y);
}

bool32 TexturePixelFree(int X, int Y)
{
    return *(int32_t *)&TextureData[(X+Y*TEXTURE_WIDTH)*4]==0xdeadbeef;
}

TexturePosition GetAvailableTextureLocation(int Width, int Height)
{
    //TODO(Christof): Store first free pixel for slight speed improvement
    for(int Y=0;Y<TEXTURE_WIDTH;Y++)
    {
	for(int X=0;X<TEXTURE_WIDTH;X++)
	{
	    if(Y+Height>=TEXTURE_HEIGHT || X+ Width >= TEXTURE_WIDTH)
		continue;
	    //TODO(Christof): figure out a better way to do this without goto
	    for(int TexY=0;TexY<Height;TexY++)
	    {
		for(int TexX=0;TexX<Width;TexX++)
		{
		    if(!TexturePixelFree(X+TexX,Y+TexY))
		    {
			goto next;
		    }
		}
	    }
	    return {X,Y};
	    next:
	    continue;
	}
    }
    return {-1,-1};
}

