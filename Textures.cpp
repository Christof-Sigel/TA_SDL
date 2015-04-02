

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

struct Texture
{
    char Name[32];
    int FrameNumber;
    float U,V;
    float Width,Height;
    FILE_GafEntry * From;
};

const int TEXTURE_WIDTH=2048;
const int TEXTURE_HEIGHT=2048;
const int MAX_NUMBER_OF_TEXTURE=1024;
Texture Textures[MAX_NUMBER_OF_TEXTURE];
int NextTexture=0;
char TextureData[TEXTURE_HEIGHT*TEXTURE_WIDTH*4];

char PaletteData[1024];
bool32 PaletteLoaded=0;

TexturePosition GetAvailableTextureLocation(int Width, int Height);
void SetTextureLocationUsed(int X, int Y, int Width, int Height);

bool32 NamesMatch(const char * NameToFind, const char * TextureName)
{
    while(*TextureName && *NameToFind)
    {
	char pcomp=*NameToFind, dcomp=*TextureName;
	if(pcomp >='a' && pcomp <='z')
	    pcomp+='A'-'a';
	if(dcomp >='a' && dcomp <='z')
	    dcomp+='A'-'a';
	if(dcomp!=pcomp)
	    return 0;
	
	NameToFind++;
	TextureName++;
    }
    return *TextureName == *NameToFind;
}

Texture * GetTexture(const char * Name,int FrameNumber)
{
    //TODO(Christof): some better storage/retrieval mechanism for texture lookup?
    for(int i=0;i<NextTexture;i++)
    {
	if(Textures[i].FrameNumber == FrameNumber && NamesMatch(Name,Textures[i].Name))
	    return &Textures[i];
    }
    return 0;
}

void LoadGafFrameEntry(char * Buffer, int Offset)
{
    FILE_GafEntry * Entry = (FILE_GafEntry*)(Buffer +Offset);
    FILE_GafFrameEntry * FrameEntries=(FILE_GafFrameEntry *)(Buffer + Offset + sizeof(*Entry));

    for(int i=0;i<Entry->NumberOfFrames;i++)
    {
	FILE_GafFrameData * Frame = (FILE_GafFrameData *)(Buffer + FrameEntries[i].FrameInfoOffset);
	if(Frame->NumberOfSubframes!=0)
	{
	    //NOTE: no textures from totala1.hpi have subframes, ignoring for now
	    LogError("%s(%d) has %d subframes",Entry->Name,i,Frame->NumberOfSubframes);
	    continue;
	}
	Texture * Texture=GetTexture(Entry->Name,i);
	if(Texture)
	{
	    // LogInformation("Skipping %s(%d of %d) as already loaded from %s(%d)",Entry->Name,i,Entry->NumberOfFrames,Texture->From->Name,Texture->From->NumberOfFrames);
	    continue;
	}
	
	//NOTE: Many textures have non-zero X and/or Y, perhaps this influences texture coord gen in some way? rendering in the past has ignored these values and seemed fine, so going to just ignore them for now

	TexturePosition PositionToStore = GetAvailableTextureLocation(Frame->Width,Frame->Height);
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
	    //NOTE: Move this out if we ever start handling Compressed textures
	    memcpy(Textures[NextTexture].Name,Entry->Name,32);
	    Textures[NextTexture].FrameNumber = i;
	    Textures[NextTexture].U=PositionToStore.X/float(TEXTURE_WIDTH);
	    Textures[NextTexture].V=PositionToStore.Y/float(TEXTURE_HEIGHT);
	    Textures[NextTexture].Width=Frame->Width/float(TEXTURE_WIDTH);
	    Textures[NextTexture].Height=Frame->Height/float(TEXTURE_HEIGHT);
	    Textures[NextTexture].From = Entry;
	    NextTexture++;
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
void LoadPalette()
{
    for(int i=0;i<GlobalArchiveCollection.NumberOfFiles;i++)
    {
    HPIEntry Palette = FindHPIEntry(&GlobalArchiveCollection.Files[i],"palettes/PALETTE.PAL");
    if(!Palette.Name)
    {
	LogWarning("No Palette found in %s",GlobalArchiveCollection.Files[i].Name);
    }
    else if(Palette.IsDirectory)
    {
	LogWarning("Directory found while trying to load palette");
    }
    else
    {
	PaletteLoaded=1;
	LoadHPIFileEntryData(Palette,PaletteData);
	return;
    }
    }
}


void LoadTextures(HPIFile* HPI)
{
    if(!PaletteLoaded)
    {
	LoadPalette();
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
    LogDebug("Loading %d textures from %s",Textures.Directory.NumberOfEntries,HPI->Name);
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
}


GLuint UnitTexture;

bool32 LoadAllTextures()
{
    //Clear Texture data so we can lookup whether a space is free
    int size=TEXTURE_WIDTH *TEXTURE_HEIGHT*4/4;
    int32_t * ClearDataPointer=(int32_t *)TextureData;
    for(int i=0;i<size;i++)
	ClearDataPointer[i]=0xdeadbeef;


    for(int i=0;i<GlobalArchiveCollection.NumberOfFiles;i++)
    {
	if(GlobalArchiveCollection.Files[i].Name)
	{
	    LoadTextures(&GlobalArchiveCollection.Files[i]);
	}
    }
    glGenTextures(1,&UnitTexture);
    glBindTexture(GL_TEXTURE_2D,UnitTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TEXTURE_WIDTH,TEXTURE_HEIGHT,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
    return 1;

}

inline bool32 TexturePixelFree(int X, int Y)
{
    return *(int32_t *)&TextureData[(X+Y*TEXTURE_WIDTH)*4]==0xdeadbeef;
}

TexturePosition FirstFreeTexture={0,0};

TexturePosition GetAvailableTextureLocation(int Width, int Height)
{
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
		    if(!TexturePixelFree(X+TexX,Y+TexY))
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

