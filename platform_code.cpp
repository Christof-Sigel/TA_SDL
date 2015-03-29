#ifdef __LINUX__
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#else

#endif


#define CONSOLE_COLOR_NORMAL  "\x1B[0m"
#define CONSOLE_COLOR_RED  "\x1B[31m"
#define CONSOLE_COLOR_GREEN  "\x1B[32m"
#define CONSOLE_COLOR_YELLOW  "\x1B[33m"
#define CONSOLE_COLOR_BLUE  "\x1B[34m"
#define CONSOLE_COLOR_MAGENTA  "\x1B[35m"
#define CONSOLE_COLOR_CYAN  "\x1B[36m"
#define CONSOLE_COLOR_WHITE  "\x1B[37m"
#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_BOLD "\x1B[1m"
#define CONSOLE_COLOR_WHITE_BOLD "\x1B[1;37m"

const char * LogError[]={CONSOLE_COLOR_BOLD CONSOLE_COLOR_RED "ERROR" ,
			 CONSOLE_COLOR_YELLOW "WARNING" ,
			 CONSOLE_COLOR_BLUE "INFORMATION",
			 CONSOLE_COLOR_GREEN "DEBUG"};
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
    printf("%s:"CONSOLE_COLOR_WHITE_BOLD" %s "CONSOLE_COLOR_RESET"line "CONSOLE_COLOR_WHITE_BOLD"%d"CONSOLE_COLOR_RESET" - %s : %s\n",LogError[loglevel],file,line,caller,tmp);
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
    int64_t FileSize;
};

MemoryMappedFile MemoryMapFile(const char * FileName)
{
    MemoryMappedFile MMFile={0};
#ifdef __LINUX__
    struct stat64 filestats;
    
    if(stat64(FileName,&filestats)==-1)
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

    DWORD high=0;
    DWORD low=GetFileSize(MMFile.File,&high);
    MMFile.FileSize = low | (high<<32);
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

#include <png.h>
#include "zlib.h"

void SaveDataToPng(char * ImageData, char * FileName, int Width, int Height)
{
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,NULL, NULL);
    if (!png_ptr)
    {
        LogError("Could not create PNG write stucture for %s",FileName);
	return;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
        LogError("Could not create PNG info stucture for %s",FileName);
	return;
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        LogError("Could not set PNG jmp ptr for %s",FileName);
	return;
    }
    FILE *fp = fopen(FileName, "wb");
    if (!fp)
    {
        LogError("Failed to open %s",FileName);
	return;
    }
    png_init_io(png_ptr, fp);
    png_set_filter(png_ptr, 0,PNG_ALL_FILTERS);
    png_set_compression_level(png_ptr,Z_DEFAULT_COMPRESSION);
    png_set_compression_strategy(png_ptr,Z_DEFAULT_STRATEGY);
    png_set_IHDR(png_ptr, info_ptr, Width, Height,8,PNG_COLOR_TYPE_RGB , PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT) ;
    png_write_info(png_ptr, info_ptr);
    png_write_flush(png_ptr);

    unsigned char ** row_pointers =new unsigned char*[Height];
    for (int i=0; i<Height; i++)
        row_pointers[i]=(unsigned char *)&(ImageData[(Height-1-i)*Width*3]);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    delete[] row_pointers;

}
