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
}

HPIFile * HPIFile::GetFile(std::string filename)
{
    if(Name.compare(filename)==0)
	return this;
    return nullptr;
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
