enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFORMATION,
    LOG_DEBUG
};
#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

const int LOG_LEVEL=LOG_DEBUG; //controls up to what level of errors is logged
#define Log(ll,fmt,...) __LOG(ll,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)
#define LogError(fmt,...)__LOG(LOG_ERROR,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__) 
#define LogWarning(fmt,...) __LOG(LOG_WARNING,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__) 
#define LogInformation(fmt,...) __LOG(LOG_INFORMATION,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)
#define LogDebug(fmt,...) __LOG(LOG_DEBUG,fmt,__func__,__LINE__,__FILE__,##__VA_ARGS__)

void __LOG(int loglevel,const char * fmt, const char* caller , int line,const char * file,...);
