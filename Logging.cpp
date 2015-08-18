#include "Logging.h"
#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef __WINDOWS__
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#ifdef __LINUX__
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
#else
#define CONSOLE_COLOR_NORMAL
#define CONSOLE_COLOR_RED
#define CONSOLE_COLOR_GREEN
#define CONSOLE_COLOR_YELLOW
#define CONSOLE_COLOR_BLUE
#define CONSOLE_COLOR_MAGENTA
#define CONSOLE_COLOR_CYAN
#define CONSOLE_COLOR_WHITE
#define CONSOLE_COLOR_RESET
#define CONSOLE_COLOR_BOLD
#define CONSOLE_COLOR_WHITE_BOLD
#endif

#define internal static

internal const char * LogError[]={CONSOLE_COLOR_BOLD CONSOLE_COLOR_RED "ERROR" ,
			 CONSOLE_COLOR_YELLOW "WARNING" ,
			 CONSOLE_COLOR_BLUE "INFORMATION",
			 CONSOLE_COLOR_GREEN "DEBUG"};

extern void __LOG(int loglevel,const char * fmt, const char* caller , int line,const char * file,...)
{
    const int MAX_LOG_STRING = 256;
    if (loglevel>LOG_LEVEL)
        return;
    va_list argptr;
    va_start(argptr,file);

    char tmp[MAX_LOG_STRING];
    va_end(argptr);
    va_start(argptr,file);
    vsnprintf(tmp,MAX_LOG_STRING,fmt,argptr);
    //TODO(Christof): Update this to use a log file at some point
    printf("%s:" CONSOLE_COLOR_WHITE_BOLD " %s " CONSOLE_COLOR_RESET "line " CONSOLE_COLOR_WHITE_BOLD "%d" CONSOLE_COLOR_RESET " - %s : %s\n",LogError[loglevel],file,line,caller,tmp);
    va_end(argptr);
}
#pragma clang diagnostic pop
