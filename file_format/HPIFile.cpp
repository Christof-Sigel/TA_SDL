#include "HPIFile.hpp"
#include <iostream>


#ifdef __LINUX__
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#else

#endif

#include <errno.h>
#include <string.h>


const int HPI_HAPI_MARKER=0x49504148;//"HAPI"
const int HPI_CHUNK_MARKER=0x48535153;//"SQSH"


const std::string CompressionTypeString[]={"NONE","LZ77","Zlib"};


void HPIFile::Print(std::string path)
{
    std::cout<<path<<Name<<std::endl;
}

HPIFile::HPIFile(HPI * cont, int Offset, unsigned char * Data,std::string name)
{
    Container=cont;
    DataOffset=*(int32_t*)(&Data[Offset]);
    FileSize=*(int32_t*)(&Data[Offset+4]);
    Name=name;
    switch(Data[Offset+8])
    {
    case 0:
	Compression=NONE;
	break;
    case 1:
	Compression=LZ77;
	break;
    case 2:
	Compression=ZLib;
	break;
    default:
	std::cout<<"Unknown Compression type: "<<(int)(Data[Offset+8])<<" in file "<<Name<<std::endl;
	break;
    }
    //std::cout<<Name<<" : "<<CompressionTypeString[Compression]<<std::endl;
}

HPIFile * HPIFile::GetFile(std::string filename)
{
    if(Name.compare(filename)==0)
	return this;
    return nullptr;
}

void LZ77Decompress(unsigned char * source, unsigned char * dest);

/** this assumes dest is either null or the correct size, since there is no way to check size
 *
 *
 *
 */
const int CHUNK_SIZE=65536;
int HPIFile::GetData(unsigned char * dest)
{
    if(dest==nullptr)
	return FileSize;
    if(Compression == NONE)
    {
	Container->Decrypt(dest,DataOffset,FileSize);
	return FileSize;
    }
    if(Compression==LZ77 || Compression==ZLib)
    {
	int NumChunks = FileSize/CHUNK_SIZE;
	if(FileSize % CHUNK_SIZE != 0)
	    NumChunks++;
	
	int32_t ChunkSize[NumChunks];
	Container->Decrypt((unsigned char*)ChunkSize,DataOffset,NumChunks*4);
	int ChunkOffset=DataOffset+NumChunks*4;
	
	for(int i=0;i<NumChunks;i++)
	{
	    unsigned char * ChunkData=new unsigned char[ChunkSize[i]];
	    Container->Decrypt(ChunkData,ChunkOffset,ChunkSize[i]);
	    ChunkOffset+=ChunkSize[i];
	    
	    if(*(int32_t*)ChunkData != HPI_CHUNK_MARKER)
	    {
		std::cout<<"Chunk marker not found in "<<Name<<std::endl;
		delete [] ChunkData;
		return -1;
	    }
	    if(ChunkData[5]!=Compression)
	    {
		std::cout<<"Chunk compression method does not match file compression method in "<<Name<<std::endl;
		delete [] ChunkData;
		return -1;
	    }
	    int CompressedSize=*((int32_t*)&ChunkData[7]);
	    int32_t CheckSum=*(int32_t*)&ChunkData[15];
	    int32_t CalcCheckSum=0;
	    for(int c=0;c<CompressedSize;c++)
	    {
		CalcCheckSum+=ChunkData[c+19];
	    }
	    if(ChunkData[6])
	    {
		//Encrypted again!
		for(unsigned int e=0;e<CompressedSize;e++)
		{
		    ChunkData[e+19]=(ChunkData[e+19] - e)^e;
		}
	    }
	    int UncompressedSize=*((int32_t*)&ChunkData[11]);
	    if(Compression==LZ77)
	    {
		LZ77Decompress(&ChunkData[19],&dest[i*CHUNK_SIZE]);
	    }
	    else
	    {
		//ZLibDecompress(
	    }

	    
	    delete [] ChunkData;
	    if(CheckSum!=CalcCheckSum)
	    {
		std::cout<<"Calculated and stored CheckSum do not match "<<CalcCheckSum<<" != "<<CheckSum<<std::endl;
		return -1;
	    }
	}
	
	return FileSize;
    }
    return -1;
}

const int LZ77_WINDOW_SIZE=4096;
const int LZ77_LENGTH_MASK=0x0f;
void LZ77Decompress(unsigned char * source, unsigned char * dest)
{
    unsigned char Window[LZ77_WINDOW_SIZE];
    
    int srcIndex=0;
    int destIndex=0;
    int windowIndex=1;
    while(true)
    {
	int tag=source[srcIndex++];
	int mask=1;
	for(int MaskCount=0;MaskCount<8;MaskCount++,mask*=2)
	{
	    if((tag & mask) == 0)
	    {
		//read a byte, put it in window and buffer
		dest[destIndex++]=source[srcIndex];
		Window[windowIndex++]=source[srcIndex++];
		if(windowIndex>=LZ77_WINDOW_SIZE)
		    windowIndex=0;
	    }
	    else
	    {
		//read two bytes, lower 4 bytes are length, upper 12 are offset into window
		
		int offset=(*(uint16_t*)&source[srcIndex])>>4;
		int length=(*(uint16_t*)&source[srcIndex])&LZ77_LENGTH_MASK;
		length+=2;
		//std::cout<<offset<<" for "<<length<<std::endl;
		if(offset==0)
		{
		    return;
		}
		
		
		srcIndex+=2;
	
		for(int readCount=0;readCount<length;readCount++)
		{
		    dest[destIndex++]=Window[offset];
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

class HPIDirectory
{
public:
    HPIDirectory(HPI * cont,int Offset,unsigned char * Data,std::string name)
    {
	Container=cont;
	Name=name;
	int NumDirectoryEntries=*(int32_t*)(&Data[Offset]);
	int DirectoryEntryOffset=*(int32_t*)(&Data[Offset+4]);
	NumFiles=NumDirectories=0;
	 
	for(int i=0;i<NumDirectoryEntries;i++)
	{
	    int Flag=(int)Data[DirectoryEntryOffset+i*9+8];
	    if(Flag==0)
		NumFiles++;
	    else
		NumDirectories++;
	}

	Files=new HPIFile *[NumFiles];
	Directories= new HPIDirectory *[NumDirectories];
	int fileIndex=0;
	int dirIndex=0;
	for(int i=0;i<NumDirectoryEntries;i++)
	{
	    int32_t NameOffset=*(int32_t*)(&Data[DirectoryEntryOffset+i*9]);
	    int DataOffset=*(int32_t*)(&Data[DirectoryEntryOffset+i*9+4]);
	    int Flag=(int)Data[DirectoryEntryOffset+i*9+8];
	    std::string Name((char *)(&Data[NameOffset]));
	    if(Flag==1)
	    {
		Directories[dirIndex++]=new HPIDirectory(cont,DataOffset,Data,Name);
	    }
	    else
	    {
		Files[fileIndex++]= new HPIFile(cont,DataOffset,Data,Name);
	    }
	}
    }

    HPIFile * GetFile(std::string filename)
    {
	if(filename.length()<=Name.length())
	    return nullptr;
	if(filename.compare(0,Name.length(),Name)!=0)
	    return nullptr;
	std::string newpath=filename.substr(Name.length()+1,std::string::npos);
	HPIFile * file=nullptr;
	for(int i=0;i<NumFiles;i++)
	{
	    file=Files[i]->GetFile(newpath);
	    if(file)
		return file;
	}
	for(int i=0;i<NumDirectories;i++)
	{
	    file=Directories[i]->GetFile(newpath);
	    if(file)
		break;
	}
	return file;
    }
    
    ~HPIDirectory()
    {
	for(int i=0;i<NumFiles;i++)
	    delete Files[i];
	delete [] Files;
	for(int i=0;i<NumDirectories;i++)
	    delete Directories[i];
	delete [] Directories;
    }

    void Print(std::string path)
    {
	std::string currentPath=path+Name+"/";
	std::cout<<currentPath<<std::endl;
	for(int i=0;i<NumFiles;i++)
	{
	    Files[i]->Print(currentPath);
	}
	for(int i=0;i<NumDirectories;i++)
	{
	    Directories[i]->Print(currentPath);
	}
    }

private:
    HPI * Container;
    int NumFiles;
    int NumDirectories;
    HPIFile ** Files;
    HPIDirectory ** Directories;
    std::string Name;
};


HPIFile * HPI::GetFile(std::string filename)
{
    return Directory->GetFile(filename);
}


void HPI::Print()
{
    Directory->Print(std::string(""));
}

HPI::HPI(std::string filename)
{
    FileName=filename;
    Directory=nullptr;
    #ifdef __LINUX__
    struct stat filestats;
    
    if(stat(filename.c_str(),&filestats)==-1)
	throw IOFail(std::string(strerror(errno)));
    FileSize=filestats.st_size;
    
    File=open(filename.c_str(),O_RDONLY);
    if(File==-1)
	throw IOFail(std::string(strerror(errno)));

    MMapBuffer=static_cast<unsigned char *>(mmap(MMapBuffer,FileSize,PROT_READ,MAP_SHARED,File,0));
    #else
    #ifdef __WINDOWS__
    OFSTRUCT of;
    File=(HANDLE)OpenFile(FileName.c_str(),&of,OF_READ);
    if(File==(HANDLE)HFILE_ERROR)
    {
	throw IOFail(std::string("Could not open file: ")+FileName);
    }
    FileSize=GetFileSize(File,NULL);
    MMFile=CreateFileMapping(File,NULL,PAGE_READONLY,0,0,NULL);
    if(!MMFile)
    {
	throw IOFail(std::string("Could not map file: ")+FileName);
    }
    MMapBuffer=static_cast<unsigned char *>(MapViewOfFile(MMFile,FILE_MAP_READ,0,0,0));
    #endif
    #endif

    if(*(int32_t*)MMapBuffer != HPI_HAPI_MARKER)
    {
	std::cout<<"Marker not found"<<std::endl;
	return;
    }

    DirectorySize=*(int32_t*)(&MMapBuffer[8]);
    int32_t HeaderKey=*(int32_t*)(&MMapBuffer[12]);
    DecryptionKey = ~((HeaderKey*4)|(HeaderKey>>6));
    DirectoryStart=*(int32_t*)(&MMapBuffer[16]);

    unsigned char * DirectoryBuffer=new unsigned char[DirectorySize+DirectoryStart];

    memset(DirectoryBuffer,0,DirectoryStart);
    
    Decrypt(&DirectoryBuffer[DirectoryStart],DirectoryStart,DirectorySize);

    Directory=new HPIDirectory(this,DirectoryStart,DirectoryBuffer,std::string(""));

    delete [] DirectoryBuffer;
}


void HPI::Decrypt(unsigned char * destination, int start, int len)
{
    for(int i=0;i<len;i++)
    {
	destination[i]=((i+start)^DecryptionKey)^ ~(MMapBuffer[i+start]);
    }
}


HPI::~HPI()
{
    #ifdef __WINDOWS__
    UnmapViewOfFile(MMapBuffer);
    CloseHandle(MMFile);
    CloseHandle(File);
    #else
    close(File);
    munmap(MMapBuffer,FileSize);
    #endif
    if(Directory != nullptr)
	delete Directory;
}
