#ifndef __CSGL_TRIANGLE_MESH
#define __CSGL_TRIANGLE_MESH

#include "Shader.hpp"
#include <string.h>

class TriangleMesh
{
public:
    TriangleMesh(GLuint numvertices,GLfloat Vertices[],GLfloat Normals[]);
    ~TriangleMesh();
    void AddRef();
    int RemoveRef();
    void Render();
    void SetMesh();
private:
    GLuint VertexArrayObject;
    GLuint VertexBuffer,VertexIndexBuffer;
    int RefCount;
    GLuint NumVertices;

};


#endif
