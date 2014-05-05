#ifndef __3DO_FILE_HPP__
#define __3DO_FILE_HPP__
#include "../lib/TriangleMesh.hpp"


class c3DOFile
{
public:
    c3DOFile(unsigned char * buffer);

private:
    c3DOFile ** Children;
    
};




#endif
