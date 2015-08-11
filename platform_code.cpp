
#include "Logging.h"




#ifdef __WINDOWS__
s64  PerformaceCounterFrequency;
#endif

s64  GetTimeMillis(u64  PerformaceCounterFrequency)
{
    return SDL_GetPerformanceCounter()/(PerformaceCounterFrequency/1000);
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
    s64  FileSize;
    u64  ModifiedTime;
};
inline u64  GetFileModifiedTime(const char * FileName);

MemoryMappedFile MemoryMapFile(const char * FileName)
{
    MemoryMappedFile MMFile={0};
#ifdef __LINUX__
    struct stat64 filestats;
    
    if(stat64(FileName,&filestats)==-1)
	return {0};
    MMFile.FileSize=filestats.st_size;
    MMFile.ModifiedTime=filestats.st_mtime;
    
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
	LogError("Could not get file handle for %s",FileName);
	return {0};
    }

    DWORD high=0;
    DWORD low=GetFileSize(MMFile.File,&high);
    MMFile.FileSize = low | (((s64 )high) << 32);
    MMFile.ModifiedTime = GetFileModifiedTime(FileName);
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

#ifdef __LINUX__
#include <png.h>
#endif
#include "zlib.h"

#ifdef __LINUX__
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
#endif

#ifdef __LINUX__
#include <dirent.h>
static int IsUFO( const struct dirent * file)
{
    const char * fname=(char *)file->d_name;
    while(*++fname){}

    return *(fname-4)=='.' &&(*(fname-3)=='U' || *(fname-3) == 'u')
	&&(*(fname-2)=='F' || *(fname-2) == 'f')
	&&(*(fname-1)=='O' || *(fname-1) == 'o');
}
#endif

//NOTE(Christof): These should be plenty
const int MAX_UFO_FILES = 32;
const int MAX_UFO_NAME_LENGTH = 64;

struct UFOSearchResult
{
    int NumberOfFiles;
    char FileNames[MAX_UFO_FILES][MAX_UFO_NAME_LENGTH];
};
UFOSearchResult GetUfoFiles()
{
    UFOSearchResult Result={};
#ifdef __WINDOWS__
    WIN32_FIND_DATAA ffd;
    HANDLE find=FindFirstFileA(".\\data\\*.ufo", &ffd);
    if(find==INVALID_HANDLE_VALUE)
    {
	LogError("Failed to find .ufo files in data directory");
	return Result;
    }

    do
    {
	int length=(int)strlen(ffd.cFileName)+1;
	Assert(length <= MAX_UFO_NAME_LENGTH);

	memcpy(&Result.FileNames[Result.NumberOfFiles],ffd.cFileName,length);
	Result.NumberOfFiles++;
	Assert(Result.NumberOfFiles<=MAX_UFO_FILES);
    }while(FindNextFile(find,&ffd));
    FindClose(find);
#endif
#ifdef __LINUX__
    struct dirent **eps=0;

    Result.NumberOfFiles = scandir ("./data/", &eps, IsUFO, alphasort);
    Assert(Result.NumberOfFiles < MAX_UFO_FILES);
    if (Result.NumberOfFiles >= 0)
    {
	for(int i=0;i<Result.NumberOfFiles;i++)
	{
	    int length=strlen(eps[i]->d_name)+1;
	    Assert(length <= MAX_UFO_NAME_LENGTH);

	    memcpy(&Result.FileNames[i],eps[i]->d_name,length);
	    free(eps[i]);
	}
    }
    else
    {
	LogError("Failed to find .ufo files in data directory");
    }
    if(eps){free(eps);}
#endif
    return Result;
}

inline b32 CaseInsensitiveMatch(const char * String1, const char * String2)
{
    while(*String1 && *String2)
    {
	char pcomp=*String2, dcomp=*String1;
	if(pcomp >='a' && pcomp <='z')
	    pcomp+='A'-'a';
	if(dcomp >='a' && dcomp <='z')
	    dcomp+='A'-'a';
	if(dcomp!=pcomp)
	    return 0;
	
	String2++;
	String1++;
    }
    return *String1 == *String2;
}

inline b32 NameEndsWith(const char * Name, const char * EndsWith)
{
    char * End = (char*)Name;
    char * test = (char*)EndsWith;
    while(*++End){}

    while(*++test){}

    while((*End == *test || *End + 'A'-'a' == *test || *End == * test +'A'-'a') && End>=Name && test>=EndsWith)
    {
	--End;
	--test;
    }
    return test<EndsWith;
}

inline u64  GetCurrentFileTime()
{
#ifdef __WINDOWS__
    FILETIME CurrentFileTime;
    GetSystemTimeAsFileTime(&CurrentFileTime);
    ULARGE_INTEGER FileTimeAsLargeInt;
    FileTimeAsLargeInt.LowPart = CurrentFileTime.dwLowDateTime;
    FileTimeAsLargeInt.HighPart = CurrentFileTime.dwHighDateTime;
    return FileTimeAsLargeInt.QuadPart;
#endif
#ifdef __LINUX__
    struct timespec time;
    clock_gettime(CLOCK_REALTIME,&time);
    return (time.tv_sec)+(time.tv_nsec/1000/1000/1000);
#endif
}


inline u64  GetFileModifiedTime(const char * FileName)
{
#ifdef __WINDOWS__

    FILETIME FileTime={};
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(FileName, GetFileExInfoStandard, &Data))
    {
        FileTime = Data.ftLastWriteTime;
    }
    else
    {
	LogError("Could not get file write time for %s",FileName);
    }

    ULARGE_INTEGER FileTimeAsLargeInt;
    FileTimeAsLargeInt.LowPart = FileTime.dwLowDateTime;
    FileTimeAsLargeInt.HighPart = FileTime.dwHighDateTime;
    return FileTimeAsLargeInt.QuadPart;
#endif
#ifdef __LINUX__
    struct stat64 filestats;
    
    if(stat64(FileName,&filestats)==-1)
	return {0};
  
    return filestats.st_mtime;

#endif
}

