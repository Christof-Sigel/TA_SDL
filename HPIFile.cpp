#include <stdlib.h>
#include <string.h>
#include "zlib.h"

void UnloadHPIFile(HPIFile * HPI)
{
    UnMapFile(HPI->MMFile);
}

void LoadEntries(HPIDirectoryEntry * Root, u8 * Buffer, int Offset,HPIFile * File,MemoryArena * FileArena)
{
    FILE_HPIDirectoryHeader * directory_header = (FILE_HPIDirectoryHeader *)(Buffer + Offset);

    Root->NumberOfEntries = directory_header->NumberOfEntries;
    Root->Entries = PushArray(FileArena,Root->NumberOfEntries,HPIEntry);

    FILE_HPIEntry * EntriesInFile = (FILE_HPIEntry *)(Buffer + directory_header->Offset);
    for(int EntryIndex = 0; EntryIndex < directory_header->NumberOfEntries; EntryIndex++)
    {
	Root->Entries[EntryIndex].IsDirectory = EntriesInFile[EntryIndex].Flag;
	Root->Entries[EntryIndex].ContainedInFile = File;
	int NameLength = (int)strlen((char*) (Buffer + EntriesInFile[EntryIndex].NameOffset))+1;
	Root->Entries[EntryIndex].Name = PushArray(FileArena,NameLength,char);
	memcpy(Root->Entries[EntryIndex].Name,Buffer+EntriesInFile[EntryIndex].NameOffset,NameLength);
	if(Root->Entries[EntryIndex].IsDirectory)
	{
	    LoadEntries(&Root->Entries[EntryIndex].Directory,Buffer,EntriesInFile[EntryIndex].DirDataOffset,File,FileArena);
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

void DecryptHPIBuffer(HPIFile * HPI, u8 * Destination, s32 Length, s32 FileOffset)
{
    int CurrentKey=0;
    if(HPI->DecryptionKey == -1)//Header key was 0 -> No Encryption
    {
	memcpy(Destination,&HPI->MMFile.MMapBuffer[FileOffset],Length);
	return;
    }
    for(int BufferIndex=0;BufferIndex < Length; BufferIndex++)
    {
	CurrentKey =  (BufferIndex + FileOffset) ^ HPI->DecryptionKey;
	Destination[BufferIndex] = (u8 )(CurrentKey ^ ~ (HPI->MMFile.MMapBuffer[BufferIndex+FileOffset]));
    }
}
    
b32 LoadHPIFile(const char * FileName, HPIFile * HPI, MemoryArena * FileArena, MemoryArena * TempArena)
{
    MemoryMappedFile HPIMemory = MemoryMapFile(FileName);
    if(HPIMemory.MMapBuffer == 0)
    {
	LogError("Could not open %s",FileName);
	return 0;
    }
	    
    FILE_HPIHeader * header = (FILE_HPIHeader *)HPIMemory.MMapBuffer;
    if(header->HPIMarker != HPI_MARKER)
    {
	LogError("%s is not a HPI file!",FileName);
	return 0;
    }
    if(header->SaveMarker == SAVE_MARKER)
    {
	LogWarning("%s is a save file",FileName);
    }

    if(header->DirectorySize > HPIMemory.FileSize)
    {
	LogError("%s has directory size %d larger than the file is %d",FileName,header->DirectorySize,HPIMemory.FileSize);
	return 0;
    }
    HPI->MMFile = HPIMemory;
    
    HPI->DecryptionKey = ~((header->HeaderKey *4) | (header->HeaderKey >> 6));
    int NameLength=(int)strlen(FileName);
    static int MaxLength=0;
    if(NameLength>MaxLength)
    {
	MaxLength=NameLength;
	LogDebug("Current Max HPI Name Length: %d",MaxLength);
    }
    HPI->Name=PushArray(FileArena,NameLength+1,char);
    memcpy(HPI->Name,FileName,NameLength);
    HPI->Name[NameLength]=0;

    u8 * DecryptedDirectory = PushArray(TempArena,header->DirectorySize,u8 );
    DecryptHPIBuffer(HPI,DecryptedDirectory,header->DirectorySize, 0);
    LoadEntries(&HPI->Root, DecryptedDirectory, header->Offset,HPI,FileArena);
    PopArray(TempArena,DecryptedDirectory,header->DirectorySize,u8 );

    return 1;
}

void DecompressZLibChunk(u8 * Source, int CompressedSize, int DecompressedSize, u8 * Destination)
{
    z_stream zs;
    int result;
    zs.next_in = (unsigned char *)Source;
    zs.avail_in = CompressedSize;
    zs.total_in = 0;
    zs.next_out = (unsigned char *)Destination;
    zs.avail_out = DecompressedSize;
    zs.total_out = 0;
    zs.msg = NULL;
    zs.state = NULL;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = NULL;
    zs.data_type = Z_BINARY;
    zs.adler = 0;
    zs.reserved = 0;
    result = inflateInit(&zs);
    if (result != Z_OK) {
	LogError("Error on inflateInit %d: Message: %s", result, zs.msg);
	return;
    }
    result = inflate(&zs, Z_FINISH);
    if (result != Z_STREAM_END) {
	LogWarning("Error on inflate %d: Message: %s", result, zs.msg);
	zs.total_out = 0;
    }
    result = inflateEnd(&zs);
    if (result != Z_OK) {
	LogError("Error on inflateEnd %d: Message: %s", result, zs.msg);
    }
}

const int LZ77_WINDOW_SIZE=4096;
const int LZ77_LENGTH_MASK=0x0f;
void DecompressLZ77Chunk(u8 * Source, int CompressedSize, int DecompressedSize, u8 * Destination)
{
    unsigned char Window[LZ77_WINDOW_SIZE];
    
    int windowIndex=1;
    int done = 0;
    while(!done)
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
		
		int offset=(*(u16 *)Source)>>4;
		int length=(*(u16 *)Source)&LZ77_LENGTH_MASK;
		length+=2;
		if(offset==0)
		{
		    done=1;
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

u8 * LoadChunk(u8 * Source, u8  * Destination)
{
    FILE_HPIChunk * header=(FILE_HPIChunk *)Source;
    if(header->Marker != CHUNK_MARKER)
    {
	LogError("chunk with incorrect marker %d",header->Marker);
	return 0;
    }
    u8 * data=Source+sizeof(FILE_HPIChunk);
    s32 ComputedChecksum=0;
    for(int i=0;i<header->CompressedSize;i++)
    {
	ComputedChecksum += (unsigned char)data[i];
    }
    if(ComputedChecksum != header->Checksum)
    {
     	LogError("Chunk Checksums do not match: calculated %d != read %d",ComputedChecksum,header->Checksum);
     	return 0;
    }
    if(header->Encrypt)
    {
	for(int i=0;i<header->CompressedSize;i++)
	    data[i]=(u8 )((data[i] - i) ^ i);
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
	LogWarning("Unknown Compression method %d in chunk",CompressionMethod);
	break;
    }
    
    return data + header->CompressedSize;
}

b32 LoadHPIFileEntryData(HPIEntry Entry, u8 * Destination, MemoryArena * TempArena)
{
    if(Entry.IsDirectory)
    {
	LogError("%s is a directory, no data to load!",Entry.Name);
	return 0;
    }
    if(!Entry.ContainedInFile)
    {
	LogError("%s has no HPI file",Entry.Name);
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
	//s32  ChunkSizes[NumChunks];
	STACK_ARRAY(ChunkSizes,NumChunks, s32);
	DecryptHPIBuffer(Entry.ContainedInFile,(u8 *)ChunkSizes,NumChunks*sizeof(s32),Entry.File.Offset);
	int ChunkDataSize=NumChunks*sizeof(FILE_HPIChunk);//size of all the headers
	int ChunkDataOffset = Entry.File.Offset + NumChunks*sizeof(s32 );
	for(int i=0;i<NumChunks;i++)
	{
	    ChunkDataSize += ChunkSizes[i];
	}
	u8 * DecryptedChunkData = PushArray(TempArena, ChunkDataSize, u8 );
//	STACK_ARRAY(DecryptedChunkData,ChunkDataSize,u8 );
	DecryptHPIBuffer(Entry.ContainedInFile,DecryptedChunkData,ChunkDataSize,ChunkDataOffset);
	u8 * DataSource=DecryptedChunkData;
	for(int i=0;i<NumChunks;i++)
	{
	    if(!DataSource)
	    {
		//something broke
		return 0;
	    }
	    DataSource=LoadChunk(DataSource,&Destination[i*CHUNK_SIZE]);
	}
	PopArray(TempArena, DecryptedChunkData, ChunkDataSize, u8 );
    }
    }
    return 1;
}

inline char * IsDirectory(char * Path, char * Directory)
{
    while(*Path && *Directory)
    {
	char pcomp=*Path, dcomp=*Directory;
	if(pcomp >='a' && pcomp <='z')
	    pcomp+='A'-'a';
	if(dcomp >='a' && dcomp <='z')
	    dcomp+='A'-'a';
	if(dcomp!=pcomp)
	    break;
	Path++;
	Directory++;
    }
    if(!*Directory && (*Path == '/' || *Path == '\\'))
	return ++Path;
    return 0;
}

HPIEntry FindHPIEntry(HPIDirectoryEntry Directory, const char * Path)
{
    for(int EntryIndex = 0;EntryIndex < Directory.NumberOfEntries ; EntryIndex++)
    {
	if(CaseInsensitiveMatch(Directory.Entries[EntryIndex].Name,(char *)Path))
	    return Directory.Entries[EntryIndex];
	if(Directory.Entries[EntryIndex].IsDirectory)
	{
	    char * NewPath= IsDirectory((char*)Path,Directory.Entries[EntryIndex].Name);
	    if(NewPath)
	    {
		return FindHPIEntry(Directory.Entries[EntryIndex].Directory,NewPath);
	    }
	}
    }
    return {0};
}

char HPIRootName [] = "Root";

HPIEntry FindHPIEntry(HPIFile * File, const char * Path)
{
    if(!*Path)
    {
	HPIEntry Root;
	Root.Name=HPIRootName;
	Root.Directory=File->Root;
	Root.IsDirectory=1;
	Root.ContainedInFile = File;
	return Root;
    }
    return FindHPIEntry(File->Root,Path);
}

const char * HPIFileNames[]={"rev31.gp3","btdata.ccx","ccdata.ccx","tactics1.hpi","tactics2.hpi",
			     "tactics3.hpi","tactics4.hpi","tactics5.hpi","tactics6.hpi","tactics7.hpi",
			     "tactics8.hpi","totala1.hpi","totala2.hpi","totala3.hpi","totala4.hpi"};
const int NUM_FIXED_FILES=sizeof(HPIFileNames)/sizeof(HPIFileNames[0]);

b32 LoadHPIFileCollection(HPIFileCollection * GlobalArchiveCollection, MemoryArena * GameArena, MemoryArena * TempArena)
{
    UFOSearchResult UfoFiles=GetUfoFiles();
    GlobalArchiveCollection->NumberOfFiles = UfoFiles.NumberOfFiles + NUM_FIXED_FILES;
    GlobalArchiveCollection->Files = PushArray(GameArena,GlobalArchiveCollection->NumberOfFiles,HPIFile);
    for(int i=0;i<GlobalArchiveCollection->NumberOfFiles;i++)
    {
	char * FileName=0;
	if(i>=UfoFiles.NumberOfFiles)
	{
	    FileName=(char*)HPIFileNames[i-UfoFiles.NumberOfFiles];
	}
	else
	{
	    FileName=UfoFiles.FileNames[i];
	}
	int size=snprintf(0,0,"data/%s",FileName)+1;
	//char temp[size];
	STACK_ARRAY(temp,size,char);
	snprintf(temp,size,"data/%s",FileName);
	//TODO(Christof): Determine if file memory stuff should go in a seperate arena
	LoadHPIFile(temp,&GlobalArchiveCollection->Files[i],GameArena,TempArena);
    }
    //UnloadUFOSearchResult(&UfoFiles);

    return 1;
}

b32 NameExistsInEntry(HPIEntry Entry, char * Name)
{
    for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
    {
	if(CaseInsensitiveMatch(Entry.Directory.Entries[i].Name,Name))
	    return 1;
    }
    return 0;
}

const int MAX_TEMP_ENTRY_LIST_FILES = 512;

HPIEntry FindEntryInAllFiles(const char * Path, HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena)
{
    HPIEntry Result={0};
    for(int ArchiveIndex=0;ArchiveIndex<GlobalArchiveCollection->NumberOfFiles;ArchiveIndex++)
    {
	HPIEntry temp=FindHPIEntry(&GlobalArchiveCollection->Files[ArchiveIndex],Path);
	if(temp.Name)
	{
	    if(!temp.IsDirectory)
	    {
		if(Result.IsDirectory)
		{
		    LogError("Found file in %s while loading directory %s",GlobalArchiveCollection->Files[ArchiveIndex].Name,Path);
		    return {0};
		}
		else
		{
		    return temp;
		}
	    }
	    else
	    {
		if(!Result.IsDirectory)
		{
		    Result.Name = temp.Name;
		    Result.IsDirectory=1;
		    Result.Directory.Entries=PushArray(TempArena,MAX_TEMP_ENTRY_LIST_FILES,HPIEntry);
		}
		for(int i=0;i<temp.Directory.NumberOfEntries;i++)
		{
		    if(!NameExistsInEntry(Result,temp.Directory.Entries[i].Name))
		    {
			Result.Directory.Entries[Result.Directory.NumberOfEntries++]=temp.Directory.Entries[i];
		    }
		    Assert(Result.Directory.NumberOfEntries < MAX_TEMP_ENTRY_LIST_FILES);
		}
	    }
	}
    }
    return Result;
}

void UnloadCompositeEntry(HPIEntry * Entry,MemoryArena * TempArena)
{
    if(Entry && Entry->IsDirectory)
    {
	PopArray(TempArena,Entry->Directory.Entries,MAX_TEMP_ENTRY_LIST_FILES,HPIEntry);
    }
}

void UnloadHPIFileCollection(HPIFileCollection * GlobalArchiveCollection)
{
    for(int i=0;i<GlobalArchiveCollection->NumberOfFiles;i++)
    {
	UnloadHPIFile(&GlobalArchiveCollection->Files[i]);
    }
    GlobalArchiveCollection->NumberOfFiles=0;
    GlobalArchiveCollection->Files=0;
}
