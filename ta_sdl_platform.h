#ifdef __LINUX__
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#else
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

typedef int32_t bool32;
