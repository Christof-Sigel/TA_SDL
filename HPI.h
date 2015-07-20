enum Compression_Type
{
    COMPRESSION_NONE=0,
    COMPRESSION_LZ77,
    COMPRESSION_ZLIB
};

const int32_t SAVE_MARKER= 'B' << 0 | 'A' <<8 | 'N' << 16 | 'K' <<24;
const int32_t HPI_MARKER= 'H' << 0 | 'A' <<8 | 'P' <<16 | 'I' << 24;
const int32_t CHUNK_MARKER = 'S' << 0 | 'Q' <<8 | 'S' <<16 | 'H' << 24;
const int32_t CHUNK_SIZE = 65536;


struct HPIFileEntry
{
    int Offset;
    int FileSize;
    Compression_Type Compression;
};

struct HPIDirectoryEntry
{
    int NumberOfEntries;
    struct HPIEntry * Entries;
};

struct HPIEntry
{
    char * Name;
    bool32 IsDirectory;
    struct HPIFile * ContainedInFile;
    union
    {
	HPIFileEntry File;
	HPIDirectoryEntry Directory;
    };
};


struct HPIFile
{
    MemoryMappedFile MMFile;
    HPIDirectoryEntry Root;
    int32_t DecryptionKey;
    char * Name;
};

struct HPIFileCollection
{
    //TODO(Christof): track directory(s?) as well
    int NumberOfFiles;
    HPIFile * Files;
};



