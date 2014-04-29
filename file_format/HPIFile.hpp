#ifndef __HPI_FILE_HPP__
#define __HPI_FILE_HPP__


#include <string>


class HPIFile
{
public:
    HPIFile(std::string filename);

private:
    unsigned char * MMapBuffer;
    unsigned int FileSize;
    int File;
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
