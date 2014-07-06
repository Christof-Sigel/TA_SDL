#ifndef __GAF_FILE_HPP__
#define __GAF_FILE_HPP__

#include <string>
#include <GL/glew.h>


struct Gaf
{
    int32_t NumEntries;
    struct GafEntry * Entries;
    Gaf(unsigned char * buffer);
    GLuint GetGLTexture(std::string Name);
};




#endif
