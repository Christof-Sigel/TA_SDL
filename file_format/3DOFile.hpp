#ifndef __3DO_FILE_HPP__
#define __3DO_FILE_HPP__

#include <GL/glew.h>
#include <string>

class Unit3DObject
{
public:
    Unit3DObject(unsigned char * buffer, int offset=0);

private:
    Unit3DObject ** Children;
    GLuint * Textures;
    GLuint * VertexArrayObjects;//Number of VAOs is determined by the number of textures as triangles/colors/uv coords will be stored per texture to reduce the number of texture binds
    //this may not be ideal, may need to come up with a better solution later, but will do for now
    GLuint SelectionVAO;
    int NumTextures;
    int NumChildren;
    std::string Name;
};




#endif
