

#pragma pack(push,1)

struct FILE_TNTHeader
{
    uint32_t IDVersion;
    uint32_t Width;
    uint32_t Height;
    uint32_t MapDataOffset;
    uint32_t MapAttributeOffset;
    uint32_t TileGraphicsOffset;
    uint32_t NumberOfTiles;
    uint32_t NumberOfFeatures;
    uint32_t FeaturesOffset;
    uint32_t SeaLevel;
    uint32_t MiniMapOffset;
    uint32_t Unknown1;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
    uint32_t pad4;
};

struct FILE_TNTAttribute
{
    uint8_t Height;
    uint16_t FeatureIndex;
    uint8_t pad;//probably padding, not entirely clear
};

struct FILE_TNTTile
{
    uint8_t TileData[32*32];
};

struct FILE_TNTFeature
{
    uint32_t Index;//this apparently matches the index used to get to this entry?
    char name[128];
};
    
#pragma pack(pop)
struct TAMap
{
    GLuint MapVertexBuffer, MinimapTexture;
    void Render()
    {
	
    }

    void RenderMiniMap()
    {
	//TODO(Christof): Update UIElements to have background (will use this here)
    }
};


const int TNT_HEADER_ID=0x2000;


TAMap LoadTNTFromBuffer(char * Buffer)
{
    FILE_TNTHeader * Header = (FILE_TNTHeader *)Buffer;
    if(Header->IDVersion != TNT_HEADER_ID)
    {
	return {0};
    }
    uint16_t * TileIndices = (uint16_t*)(Buffer + Header->MapDataOffset);//these map to 32x32 tiles NOT 16x16 half tiles
    int NumberOfHalfTiles = Header->Width*Header->Height;
    int NumberOfTiles = NumberOfHalfTiles /4 ;//heights and features defined on 16x16 tiles, graphics tiles are 32x32
    FILE_TNTAttribute * Attributes=(FILE_TNTAttribute *)(Buffer + Header->MapAttributeOffset);//half tiles
    FILE_TNTTile * Tiles=(FILE_TNTTile *)(Buffer + Header->TileGraphicsOffset);
    FILE_TNTFeature * Features=(FILE_TNTFeature*)(Buffer + Header->FeaturesOffset);

    int MinimapWidth, MinimapHeight;
    MinimapWidth = *(uint32_t*)(Buffer + Header->MiniMapOffset);
    MinimapWidth = *(uint32_t*)(Buffer + Header->MiniMapOffset+4);
    uint8_t * MinimapData = (uint8_t *)(Buffer + Header->MiniMapOffset + 8);

    //TODO(Christof): TA Apparently uses 0xDD to denote transparency, need to deal with this here
    
}
