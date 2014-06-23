#ifndef __HPI_FILE_HPP__
#define __HPI_FILE_HPP__


#include <SDL2/SDL_platform.h>
#include <string>
#include <stdint.h>

#ifdef __WINDOWS__
#include <windows.h>
#endif

enum HPICompressionType{NONE,LZ77,ZLib};

class HPI
{
public:
    HPI(std::string filename);
    ~HPI();
    void Print();
    class HPIFile * GetFile(std::string filename);
    class HPIDirectory * GetDirectory(std::string filename);
private:
    friend class HPIFile;
    unsigned char * MMapBuffer;
    unsigned int FileSize;
    class HPIDirectory * Directory;
    
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

class HPIFile
{
private:
    HPI * Container;
    std::string Name;
    int DataOffset;
    int FileSize;
    HPICompressionType Compression;
public:
    void Print(std::string path);
    HPIFile(HPI * cont, int Offset, unsigned char * Data,std::string name);
    HPIFile * GetFile(std::string filename);
    int GetData(unsigned char * data);
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

class HPIDirectory
{
public:
    HPIDirectory(HPI * cont,int Offset,unsigned char * Data,std::string name);
    HPIDirectory * GetDirectory(std::string filename);
    HPIFile * GetFile(std::string filename);
    
    ~HPIDirectory();

    void Print(std::string path);

private:
    HPI * Container;
    int NumFiles;
    int NumDirectories;
    HPIFile ** Files;
    HPIDirectory ** Directories;
    std::string Name;
};



#else
class HPIFile;
#endif
