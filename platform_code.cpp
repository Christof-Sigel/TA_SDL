internal u64 GetTimeMillis(u64 PerformaceCounterFrequency)
{
    return SDL_GetPerformanceCounter()/(PerformaceCounterFrequency/1000);
}

inline u64  GetFileModifiedTime(const char * FileName);

internal MemoryMappedFile MemoryMapFile(const char * FileName)
{
    MemoryMappedFile MMFile={};
#ifdef __LINUX__
    struct stat64 filestats;

    if(stat64(FileName,&filestats)==-1)
	return {};
    MMFile.FileSize=(u64)filestats.st_size;
    MMFile.ModifiedTime=(u64)filestats.st_mtime;
    
    MMFile.File=open(FileName,O_RDONLY);
    if(MMFile.File==-1)
	return {};

    MMFile.MMapBuffer=static_cast<unsigned char *>(mmap(MMFile.MMapBuffer,MMFile.FileSize,PROT_READ,MAP_SHARED,MMFile.File,0));
#else
#ifdef __WINDOWS__
    MMFile.File=CreateFileA(FileName,GENERIC_READ,0,0,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
    if(MMFile.File == INVALID_HANDLE_VALUE)
    {
	LogError("Could not get file handle for %s",FileName);
	return {};
    }

    DWORD high=0;
    DWORD low=GetFileSize(MMFile.File,&high);
    MMFile.FileSize = low | (((s64 )high) << 32);
    MMFile.ModifiedTime = GetFileModifiedTime(FileName);
    MMFile.MMFile=CreateFileMapping(MMFile.File,NULL,PAGE_READONLY,0,0,NULL);
    if(!MMFile.MMFile)
    {
	return {};
    }
    MMFile.MMapBuffer=static_cast<unsigned char *>(MapViewOfFile(MMFile.MMFile,FILE_MAP_READ,0,0,0));
#endif
#endif

    return MMFile;
}

internal void UnMapFile(MemoryMappedFile MMFile)
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

internal UFOSearchResult GetUfoFiles()
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
	size_t length=strlen(ffd.cFileName)+1;
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
	    size_t length=strlen(eps[i]->d_name)+1;
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

inline u64 GetCurrentFileTime()
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
    return (u64)(time.tv_sec)+(u64)(time.tv_nsec/1000/1000/1000);
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
	return {};
  
    return (u64)filestats.st_mtime;
#endif
}
