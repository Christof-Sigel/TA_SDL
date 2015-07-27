#pragma pack(push,1)

struct FILE_Object3dHeader
{
    s32 Version;
    s32 NumberOfVertexes;
    s32 NumberOfPrimitives;
    s32 OffsetToSelectionPrimitive;//Appears to be index into primitive array
    s32 XFromParent;
    s32 YFromParent;
    s32 ZFromParent;
    s32 OffsetToObjectName;
    s32 Always_0;
    s32 OffsetToVertexArray;
    s32 OffsetToPrimitiveArray;
    s32 OffsetToSiblingObject;
    s32 OffsetToChildObject;
};

struct FILE_Object3dVertex
{
    s32 x;
    s32 y;
    s32 z;
};

struct FILE_Object3dPrimitive
{
    s32 ColorIndex;
    s32 NumberOfVertexIndexes;
    s32 Always_0;
    s32 OffsetToVertexIndexArray;
    s32 OffsetToTextureName;
    //Apparently Cavedog editor specific:
    s32 Unknown_1;
    s32 Unknown_2;
    s32 Unknown_3;    
};

struct FILE_FNT
{
    u8 Height;
    u8 Padding1[3];
    s16  CharacterOffset[255];
};



struct FILE_HPIHeader
{
    s32 HPIMarker;
    s32 SaveMarker;
    s32 DirectorySize;
    s32 HeaderKey;
    s32 Offset;
};

struct FILE_HPIDirectoryHeader
{
    s32 NumberOfEntries;
    s32 Offset;
};

struct FILE_HPIEntry
{
    s32 NameOffset;
    s32 DirDataOffset;
    char Flag;// 0 -> File,  1 -> Directory
};

struct FILE_HPIFileData
{
    s32 DataOffset;
    s32 FileSize;
    char Flag;// 0 -> File,  1 -> Directory
};

struct FILE_HPIChunk
{
    s32 Marker;
    char Unknown1;
    char CompressionMethod;
    char Encrypt;
    s32 CompressedSize;
    s32 DecompressedSize;
    s32 Checksum;
};

struct FILE_GafHeader
{
    s32 IDVersion;
    s32 NumberOfEntries;
    s32 Unknown1; //Always 0 apparently?
};

struct  FILE_GafEntry
{
    s16  NumberOfFrames;
    s16  Unknown1;//apparently always 1
    s32 Unknown2;//apprently always 0
    char Name[32];
};

struct FILE_GafFrameEntry
{
    s32 FrameInfoOffset;
    s32 Unknown1;//completely unknown
};

struct FILE_GafFrameData
{
    s16  Width;
    s16  Height;
    s16  XPos;
    s16  YPos;
    char Unknown1;//apparently always 9?
    char Compressed;
    s16  NumberOfSubframes;
    s32 Unknown2;//always 0
    s32 FrameDataOffset;
    s32 Unknow3;//completely unknown
};

struct FILE_PCXHeader
{
    u8 Manufacturer;//should always be 0x0A
    u8 Version;
    u8 Encoding;
    u8 BitsPerPlane;
    u16  XMin, YMin, XMax, YMax, VerticalDPIOrResolution, HorizontalDPIOrResolution /* this can apparently be ommitted??? */;
    u8 palette[48], reserved, ColorPlanes;
    u16  BytesPerPlaneLine, PalletteType, HorizontalScrSize, VerticalScrSize;
    u8 Pad[54];
};


struct FILE_TNTHeader
{
    u32 IDVersion;
    u32 Width;
    u32 Height;
    u32 MapDataOffset;
    u32 MapAttributeOffset;
    u32 TileGraphicsOffset;
    u32 NumberOfTiles;
    u32 NumberOfFeatures;
    u32 FeaturesOffset;
    u32 SeaLevel;
    u32 MiniMapOffset;
    u32 Unknown1;
    u32 pad1;
    u32 pad2;
    u32 pad3;
    u32 pad4;
};

struct FILE_TNTAttribute
{
    u8 Height;
    u16  FeatureIndex;
    u8 pad;//probably padding, not entirely clear
};

struct FILE_TNTTile
{
    u8 TileData[32*32];
};

struct FILE_TNTFeature
{
    u32 Index;//this apparently matches the index used to get to this entry?
    char name[128];
};

struct FILE_CobHeader
{
	s32 VersionSignature;
	s32 NumberOfScripts;
	s32 NumberOfPieces;
	s32 CodeSize;
	s32 NumberOfStatics;
	s32 Unknown_2; /* Always seems to be 0 */
	s32 OffsetToScriptCodeIndexArray;
	s32 OffsetToScriptNameOffsetArray;
	s32 OffsetToPieceNameOffsetArray;
	s32 OffsetToScriptCode;
	s32 OffsetToScriptNames; /* Always seems to point to first script name */
};

#pragma pack(pop)




