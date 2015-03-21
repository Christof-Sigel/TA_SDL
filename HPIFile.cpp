#include <stdlib.h>
#include <string.h>

enum Compression_Type
{
    COMPRESSION_NONE=0,
    COMPRESSION_LZ77,
    COMPRESSION_ZLIB
};

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
};


const int32_t SAVE_MARKER= 'B' << 0 | 'A' <<8 | 'N' << 16 | 'K' <<24;
const int32_t HPI_MARKER= 'H' << 0 | 'A' <<8 | 'P' <<16 | 'I' << 24;
const int32_t CHUNK_MARKER = 'S' << 0 | 'Q' <<8 | 'S' <<16 | 'H' << 24;
const int32_t CHUNK_SIZE = 65536;

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

struct __attribute__((__packed__)) FILE_HPIEntry
{
    int32_t NameOffset;
    int32_t DirDataOffset;
    char Flag;// 0 -> File,  1 -> Directory
};

struct __attribute__((__packed__)) FILE_HPIFileData
{
    int32_t DataOffset;
    int32_t FileSize;
    char Flag;// 0 -> File,  1 -> Directory
};

struct __attribute__((__packed__)) FILE_HPIChunk
{
    int32_t Marker;
    char Unknown1;
    char CompressionMethod;
    char Encrypt;
    int32_t CompressedSize;
    int32_t DecompressedSize;
    int32_t Checksum;
};
void DecryptHPIBuffer(HPIFile * HPI, char * Destination, int32_t Length, int32_t FileOffset);
void LoadEntries(HPIDirectoryEntry * Root, char * Buffer, int Offset, HPIFile * File);
char * LoadChunk(char * SourceBuffer, char * Destination);
void DecompressZLibChunk(char * Source, int CompressedSize, int DecompressedSize, char * Destination);
void DecompressLZ77Chunk(char * Source, int CompressedSize, int DecompressedSize, char * Destination);
    
bool32 LoadHPIFile(const char * FileName, HPIFile * HPI)
{
    MemoryMappedFile HPIMemory = MemoryMapFile(FileName);
    if(HPIMemory.FileSize == 0)
    {
	printf("Could not open %s\n",FileName);
	return 0;
    }
	    
    FILE_HPIHeader * header = (FILE_HPIHeader *)HPIMemory.MMapBuffer;
    if(header->HPIMarker != HPI_MARKER)
    {
	printf("%s is not a HPI file!\n",FileName);
	return 0;
    }
    if(header->SaveMarker == SAVE_MARKER)
    {
	printf("%s is a save file\n",FileName);
    }

    if(header->DirectorySize > HPIMemory.FileSize)
    {
	printf("%s has directory size %d larger than the file is %d\n",FileName,header->DirectorySize,HPIMemory.FileSize);
	return 0;
    }
    HPI->MMFile = HPIMemory;
    
    HPI->DecryptionKey = ~((header->HeaderKey *4) | (header->HeaderKey >> 6));

    //TODO(Christof): Figure out a memory alloc scheme, just use malloc for now
    char * DecryptedDirectory = (char *)malloc(header->DirectorySize);
    DecryptHPIBuffer(HPI,DecryptedDirectory,header->DirectorySize, 0);
    LoadEntries(&HPI->Root, DecryptedDirectory, header->Offset,HPI);

    return 1;
}

void LoadEntries(HPIDirectoryEntry * Root, char * Buffer, int Offset,HPIFile * File)
{
    FILE_HPIDirectoryHeader * directory_header = (FILE_HPIDirectoryHeader *)(Buffer + Offset);

    Root->NumberOfEntries = directory_header->NumberOfEntries;
    Root->Entries = (HPIEntry *)malloc(sizeof(HPIEntry)*Root->NumberOfEntries);

    FILE_HPIEntry * EntriesInFile = (FILE_HPIEntry *)(Buffer + directory_header->Offset);
    for(int EntryIndex = 0; EntryIndex < directory_header->NumberOfEntries; EntryIndex++)
    {
	Root->Entries[EntryIndex].IsDirectory = EntriesInFile[EntryIndex].Flag;
	Root->Entries[EntryIndex].ContainedInFile = File;
	int NameLength = strlen(Buffer + EntriesInFile[EntryIndex].NameOffset)+1;
	Root->Entries[EntryIndex].Name = (char *)malloc(NameLength);
	memcpy(Root->Entries[EntryIndex].Name,Buffer+EntriesInFile[EntryIndex].NameOffset,NameLength);
	if(Root->Entries[EntryIndex].IsDirectory)
	{
	    LoadEntries(&Root->Entries[EntryIndex].Directory,Buffer,EntriesInFile[EntryIndex].DirDataOffset,File);
	}
	else
	{
	    FILE_HPIFileData * file_data =(FILE_HPIFileData *)(Buffer + EntriesInFile[EntryIndex].DirDataOffset);
	    Root->Entries[EntryIndex].File.FileSize = file_data->FileSize;
	    Root->Entries[EntryIndex].File.Offset = file_data->DataOffset;
	    Root->Entries[EntryIndex].File.Compression = (Compression_Type)file_data->Flag;
	}
    }
}

void DecryptHPIBuffer(HPIFile * HPI, char * Destination, int32_t Length, int32_t FileOffset)
{
    int CurrentKey=0;
    for(int BufferIndex=0;BufferIndex < Length; BufferIndex++)
    {
	CurrentKey =  (BufferIndex + FileOffset) ^ HPI->DecryptionKey;
	Destination[BufferIndex] = CurrentKey ^ ~ (HPI->MMFile.MMapBuffer[BufferIndex+FileOffset]);
    }
}

bool32 LoadHPIFileEntryData(HPIEntry Entry, char * Destination)
{
    if(Entry.IsDirectory)
    {
	printf("%s is a directory, no data to load!\n",Entry.Name);
	return 0;
    }
    switch(Entry.File.Compression)
    {
    case COMPRESSION_NONE:
	DecryptHPIBuffer(Entry.ContainedInFile,Destination,Entry.File.FileSize,Entry.File.Offset);
	return 1;
    case COMPRESSION_LZ77:
    case COMPRESSION_ZLIB:
	//not entirely sure what the point of this is since each chunk also has a compression metho flag
    {
	int NumChunks = Entry.File.FileSize / CHUNK_SIZE;
	if(Entry.File.FileSize % CHUNK_SIZE)
	{
	    NumChunks++;
	}
	int32_t ChunkSizes[NumChunks];
	DecryptHPIBuffer(Entry.ContainedInFile,(char*)ChunkSizes,NumChunks*sizeof(int32_t),Entry.File.Offset);
	int ChunkDataSize=NumChunks*sizeof(FILE_HPIChunk);//size of all the headers
	int ChunkDataOffset = Entry.File.Offset + NumChunks*sizeof(int32_t);
	for(int i=0;i<NumChunks;i++)
	{
	    ChunkDataSize += ChunkSizes[i];
	}
	char DecryptedChunkData[ChunkDataSize];
	DecryptHPIBuffer(Entry.ContainedInFile,DecryptedChunkData,ChunkDataSize,ChunkDataOffset);
	char * DataSource=DecryptedChunkData;
	for(int i=0;i<NumChunks;i++)
	{
	    if(!DataSource)
	    {
		//something broke
		return 0;
	    }
	    DataSource=LoadChunk(DataSource,&Destination[i*CHUNK_SIZE]);
	}
    }
    }

    return 1;
}

char * LoadChunk(char * Source, char * Destination)
{
    FILE_HPIChunk * header=(FILE_HPIChunk *)Source;
    if(header->Marker != CHUNK_MARKER)
    {
	printf("chunk with incorrect marker %d\n",header->Marker);
	return 0;
    }
    char * data=Source+sizeof(FILE_HPIChunk);
    int32_t ComputedChecksum=0;
    for(int i=0;i<header->CompressedSize;i++)
    {
	ComputedChecksum += (unsigned char)data[i];
    }
    if(ComputedChecksum != header->Checksum)
    {
     	printf("Chunk Checksums do not match: %d != %d\n",ComputedChecksum,header->Checksum);
     	return 0;
    }
    if(header->Encrypt)
    {
	for(int i=0;i<header->CompressedSize;i++)
	    data[i]=(data[i] - i) ^ i;
    }
    Compression_Type CompressionMethod= (Compression_Type)header->CompressionMethod;
    switch(CompressionMethod)
    {
    case COMPRESSION_ZLIB:
	DecompressZLibChunk(data,header->CompressedSize,header->DecompressedSize,Destination);
	break;
    case COMPRESSION_LZ77:
	DecompressLZ77Chunk(data,header->CompressedSize,header->DecompressedSize,Destination);
	break;
    default:
	printf("Unknown Compression method %d in chunk\n",CompressionMethod);
	break;
    }
    
    return data + header->CompressedSize;
}

void DecompressZLibChunk(char * Source, int CompressedSize, int DecompressedSize, char * Destination)
{
    printf("Zlib compression not yet supported\n");
}


const int LZ77_WINDOW_SIZE=4096;
const int LZ77_LENGTH_MASK=0x0f;
void DecompressLZ77Chunk(char * Source, int CompressedSize, int DecompressedSize, char * Destination)
{
    unsigned char Window[LZ77_WINDOW_SIZE];
    
    int windowIndex=1;
    while(true)
    {
	int tag=*Source++;
	int mask=1;
	for(int MaskCount=0;MaskCount<8;MaskCount++,mask*=2)
	{
	    if((tag & mask) == 0)
	    {
		//read a byte, put it in window and buffer
		*Destination++=*Source;
		Window[windowIndex++]=*Source++;
		if(windowIndex>=LZ77_WINDOW_SIZE)
		    windowIndex=0;
	    }
	    else
	    {
		//read two bytes, lower 4 bytes are length, upper 12 are offset into window
		
		int offset=(*(uint16_t*)Source)>>4;
		int length=(*(uint16_t*)Source)&LZ77_LENGTH_MASK;
		length+=2;
		if(offset==0)
		{
		    return;
		}
		Source+=2;
		for(int readCount=0;readCount<length;readCount++)
		{
		    *Destination++=Window[offset];
		    Window[windowIndex++]=Window[offset++];
		    if(offset>=LZ77_WINDOW_SIZE)
			offset=0;
		    if(windowIndex>=LZ77_WINDOW_SIZE)
			windowIndex=0;
		}
	    }
	}
    }
}

HPIEntry FindHPIEntry(HPIDirectoryEntry Directory, char * Path)
{
    for(int EntryIndex = 0;EntryIndex < Directory.NumberOfEntries ; EntryIndex++)
    {
	if(strcmp(Directory.Entries[EntryIndex].Name,Path)==0)
	    return Directory.Entries[EntryIndex];
	else if(Directory.Entries[EntryIndex].IsDirectory)
	{
	    char * NewPath= strstr(Path,Directory.Entries[EntryIndex].Name);
	    if(NewPath)
	    {
		NewPath+=strlen(Directory.Entries[EntryIndex].Name)+1;
		printf("%s\n",NewPath);
		return FindHPIEntry(Directory.Entries[EntryIndex].Directory,NewPath);
	    }
	}
    }
    return {0};
}

HPIEntry FindHPIEntry(HPIFile File, char * Path)
{
    return FindHPIEntry(File.Root,Path);
}
