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


const int HPI_HAPI_MARKER=0x49504148;

HPIFile::HPIFile(std::string filename)
{
    FileName=filename;
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
    }

    DirectorySize=*(int32_t*)(&MMapBuffer[8]);
    int32_t HeaderKey=*(int32_t*)(&MMapBuffer[12]);
    DecryptionKey = ~((HeaderKey*4)|(HeaderKey>>6));
    DirectoryStart=*(int32_t*)(&MMapBuffer[16]);

    std::cout<<DirectorySize<<std::endl;

    unsigned char DirectoryBuffer[DirectorySize];

    Decrypt(DirectoryBuffer,DirectoryStart,DirectorySize);

    int NumDirectoryEntries=*(int32_t*)DirectoryBuffer;
    int DirectoryEntryOffset=*(int32_t*)(&DirectoryBuffer[4]);
    DirectoryEntryOffset-=DirectoryStart;
    
    
    for(int i=0;i<NumDirectoryEntries;i++)
    {
	
    }
}


void HPIFile::Decrypt(unsigned char * destination, int start, int len)
{
    for(int i=0;i<len;i++)
    {
	destination[i]=((i+start)^DecryptionKey)^ ~(MMapBuffer[i+start]);
    }
}


HPIFile::~HPIFile()
{
    #ifdef __WINDOWS__
    UnmapViewOfFile(MMapBuffer);
    CloseHandle(MMFile);
    CloseHandle(File);
    #else
    close(File);
    munmap(MMapBuffer,FileSize);
    #endif
}
