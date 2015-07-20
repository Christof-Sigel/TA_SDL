#pragma pack(push,1)

struct FILE_Object3dHeader
{
    int32_t Version;
    int32_t NumberOfVertexes;
    int32_t NumberOfPrimitives;
    int32_t OffsetToSelectionPrimitive;//Appears to be index into primitive array
    int32_t XFromParent;
    int32_t YFromParent;
    int32_t ZFromParent;
    int32_t OffsetToObjectName;
    int32_t Always_0;
    int32_t OffsetToVertexArray;
    int32_t OffsetToPrimitiveArray;
    int32_t OffsetToSiblingObject;
    int32_t OffsetToChildObject;
};

struct FILE_Object3dVertex
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct FILE_Object3dPrimitive
{
    int32_t ColorIndex;
    int32_t NumberOfVertexIndexes;
    int32_t Always_0;
    int32_t OffsetToVertexIndexArray;
    int32_t OffsetToTextureName;
    //Apparently Cavedog editor specific:
    int32_t Unknown_1;
    int32_t Unknown_2;
    int32_t Unknown_3;    
};

struct FILE_FNT
{
    uint8_t Height;
    uint8_t Padding1[3];
    int16_t CharacterOffset[255];
};



struct FILE_HPIHeader
{
    int32_t HPIMarker;
    int32_t SaveMarker;
    int32_t DirectorySize;
    int32_t HeaderKey;
    int32_t Offset;
};

struct FILE_HPIDirectoryHeader
{
    int32_t NumberOfEntries;
    int32_t Offset;
};

struct FILE_HPIEntry
{
    int32_t NameOffset;
    int32_t DirDataOffset;
    char Flag;// 0 -> File,  1 -> Directory
};

struct FILE_HPIFileData
{
    int32_t DataOffset;
    int32_t FileSize;
    char Flag;// 0 -> File,  1 -> Directory
};

struct FILE_HPIChunk
{
    int32_t Marker;
    char Unknown1;
    char CompressionMethod;
    char Encrypt;
    int32_t CompressedSize;
    int32_t DecompressedSize;
    int32_t Checksum;
};

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

struct FILE_CobHeader
{
	int32_t VersionSignature;
	int32_t NumberOfScripts;
	int32_t NumberOfPieces;
	int32_t CodeSize;
	int32_t NumberOfStatics;
	int32_t Unknown_2; /* Always seems to be 0 */
	int32_t OffsetToScriptCodeIndexArray;
	int32_t OffsetToScriptNameOffsetArray;
	int32_t OffsetToPieceNameOffsetArray;
	int32_t OffsetToScriptCode;
	int32_t OffsetToScriptNames; /* Always seems to point to first script name */
};

#pragma pack(pop)




