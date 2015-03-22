#ifdef __LINUX__
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#else

#endif

const char * LogError[]={"ERROR","WARNING","INFORMATION","DEBUG"};
enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFORMATION,
    LOG_DEBUG
};
const int LOG_LEVEL=LOG_DEBUG; //controls up to what level of errors is logged
#define Log(ll,fmt,...) __LOG(ll,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)
#define LogError(fmt,...)__LOG(LOG_ERROR,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__) 
#define LogWarning(fmt,...) __LOG(LOG_WARNING,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__) 
#define LogInformation(fmt,...) __LOG(LOG_INFORMATION,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)
#define LogDebug(fmt,...) __LOG(LOG_DEBUG,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)

void __LOG(int loglevel,const char * fmt, const char* caller , int line,const char * file,...)
{
    if (loglevel>LOG_LEVEL)
        return;
    va_list argptr;
    va_start(argptr,file);
    char * tmp;
    int size=vsnprintf(NULL, 0, fmt, argptr);
    tmp=(char*)malloc(size+1);
    va_end(argptr);
    va_start(argptr,file);
    vsnprintf(tmp,size+1,fmt,argptr);
    //TODO(Christof): Update this to use a log file at some point
    printf("%s: %s line %d - %s : %s\n",LogError[loglevel],file,line,caller,tmp);
    va_end(argptr);
    free(tmp);
}

struct MemoryMappedFile
{
#ifdef __WINDOWS__
    HANDLE MMFile;
    HANDLE File;
#else
    int File;
#endif
    unsigned char * MMapBuffer;
    unsigned int FileSize;
};

MemoryMappedFile MemoryMapFile(const char * FileName)
{
    MemoryMappedFile MMFile={0};
#ifdef __LINUX__
    struct stat filestats;
    
    if(stat(FileName,&filestats)==-1)
	return {0};
    MMFile.FileSize=filestats.st_size;
    
    MMFile.File=open(FileName,O_RDONLY);
    if(MMFile.File==-1)
	return {0};

    MMFile.MMapBuffer=static_cast<unsigned char *>(mmap(MMFile.MMapBuffer,MMFile.FileSize,PROT_READ,MAP_SHARED,MMFile.File,0));
#else
#ifdef __WINDOWS__
    OFSTRUCT of;
    MMFile.File=(HANDLE)OpenFile(FileName,&of,OF_READ);
    if(MMFile.File==(HANDLE)HFILE_ERROR)
    {
	return {0};
    }
    MMFile.FileSize=GetFileSize(MMFile.File,NULL);
    MMFile.MMFile=CreateFileMapping(MMFile.File,NULL,PAGE_READONLY,0,0,NULL);
    if(!MMFile.MMFile)
    {
	return {0};
    }
    MMFile.MMapBuffer=static_cast<unsigned char *>(MapViewOfFile(MMFile.MMFile,FILE_MAP_READ,0,0,0));
#endif
#endif

    return MMFile;
}

void UnMapFile(MemoryMappedFile MMFile)
{
#ifdef __WINDOWS__
    UnmapViewOfFile(MMFile.MMapBuffer);
    CloseHandle(MMFile.MMFile);
    CloseHandle(MMFile.File);
#else
    close(MMFile.File);
    munmap(MMFile.MMapBuffer,MMFile.FileSize);
#endif
}
    
