#ifndef __HPI_FILE_HPP__
#define __HPI_FILE_HPP__


#include <SDL2/SDL_platform.h>
#include <string>
#include <stdint.h>

#ifdef __WINDOWS__
#include <windows.h>
#endif

class HPIFile
{
public:
    HPIFile(std::string filename);
    ~HPIFile();

private:
    unsigned char * MMapBuffer;
    unsigned int FileSize;
    
#ifdef __WINDOWS__
    HANDLE MMFile;
    HANDLE File;
#else
    int File;
#endif
    
    std::string FileName;
    int DirectorySize;
    int DirectoryStart;
    int32_t DecryptionKey;
    void Decrypt(unsigned char * destination,int start, int len);
};

class IOFail : public std::exception
{
public:
    IOFail(std::string m=std::string("No error message provided")):msg(m)
    {
    }
    virtual const char * what() const throw()
    {
	return msg.c_str();
    }
    virtual ~IOFail() throw()
    {
    }
private:
    std::string msg;
};



#else
class HPIFile;
#endif
