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

void DecryptHPIBuffer(char * Source, char * Destination, int32_t Length, int32_t DecryptionKey, int32_t FileOffset);
void LoadEntries(HPIDirectoryEntry * Root, char * Buffer, int Offset, HPIFile * File);

bool32 LoadHPIFile(const char * FileName, HPIFile * HPI)
{
    MemoryMappedFile HPIMemory = MemoryMapFile(FileName);
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
    DecryptHPIBuffer((char*)HPIMemory.MMapBuffer,DecryptedDirectory,header->DirectorySize, HPI->DecryptionKey, 0);
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

void DecryptHPIBuffer(char * Source, char * Destination, int32_t Length, int32_t DecryptionKey, int32_t FileOffset)
{
    int CurrentKey=0;
    for(int BufferIndex=0;BufferIndex < Length; BufferIndex++)
    {
	CurrentKey =  (BufferIndex + FileOffset) ^ DecryptionKey;
	Destination[BufferIndex] = CurrentKey ^ ~Source[BufferIndex+FileOffset];
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
	DecryptHPIBuffer((char*)Entry.ContainedInFile->MMFile.MMapBuffer,Destination,Entry.File.FileSize,Entry.ContainedInFile->DecryptionKey,Entry.File.Offset);
	return 1;
    case COMPRESSION_LZ77:
	printf("%s is compressed using LZ77 - this is not currently implemented\n",Entry.Name);
	return 0;
    case COMPRESSION_ZLIB:
	printf("%s is compressed using zlib - this is not currently implemented\n",Entry.Name);
	return 0;
    }

    return 1;
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
