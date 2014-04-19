#include <GL/glew.h>
#include "TriangleMesh.hpp"

bool VerticesEqual(float v1[],float v2[])
{
    return v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2];
}

struct Vertex
{
    GLfloat Position[3];
    GLfloat Normal[3];
};

bool VerticesEqual(float v1[],Vertex v2)
{
    return v1[0]==v2.Position[0] && v1[1]==v2.Position[1] && v1[2]==v2.Position[2];
}

bool VerticesEqual(Vertex v1,Vertex v2)
{
    return v1.Position[0]==v2.Position[0] && v1.Position[1]==v2.Position[1] && v1.Position[2]==v2.Position[2];
}

bool VerticesAndNormalsEqual(float v1[],float n1[],Vertex v2)
{
    return v1[0]==v2.Position[0] && v1[1]==v2.Position[1] && v1[2]==v2.Position[2] && n1[0]==v2.Normal[0] && n1[1]==v2.Normal[1] && n1[2]==v2.Normal[2];
}

TriangleMesh::TriangleMesh(GLuint numvertices,GLfloat Vertices[],GLfloat Normals[]):RefCount(0),NumVertices(numvertices)
{

    GLuint * vertexindex=new GLuint[NumVertices];
    Vertex * vertices = new Vertex[NumVertices];

    GLuint NumVert=0;
    GLuint j;
    for(GLuint i=0;i<NumVertices;i++)
    {
	for(j=0;j<NumVert;j++)
	    if(VerticesAndNormalsEqual(&Vertices[i*3],&Normals[i*3],vertices[j]))
	    {
		vertexindex[i]=j;
		break;
	    }
	if(j==NumVert)
	{
	    vertices[j].Position[0]=Vertices[i*3+0];
	    vertices[j].Position[1]=Vertices[i*3+1];
	    vertices[j].Position[2]=Vertices[i*3+2];
	    vertices[j].Normal[0]=Normals[i*3+0];
	    vertices[j].Normal[1]=Normals[i*3+1];
	    vertices[j].Normal[2]=Normals[i*3+2];
	    vertexindex[i]=j;
	    NumVert++;
	}
    }

    GLuint * adjacentvertexindex=new GLuint[NumVertices*2];
    memset(adjacentvertexindex,-1,sizeof(GLuint)*NumVertices*2);
    for(GLuint i=0;i<NumVertices/3;i++)
    {
	for(GLuint k=0;k<3;k++)
	{
	    adjacentvertexindex[i*3*2+k*2]=vertexindex[i*3+k];
	    for(GLuint j=i+1;j<NumVertices/3;j++)
	    {
		for(GLuint l=0;l<3;l++)
		{
		    if((VerticesEqual(vertices[vertexindex[i*3+k]],vertices[vertexindex[j*3+l]])
			&& VerticesEqual(vertices[vertexindex[i*3+(k+1)%3]],vertices[vertexindex[j*3+(l+1)%3]]))
		       ||
		       (VerticesEqual(vertices[vertexindex[i*3+k]],vertices[vertexindex[j*3+(l+1)%3]])
			&& VerticesEqual(vertices[vertexindex[i*3+(k+1)%3]],vertices[vertexindex[j*3+l]]))
			)
		    {
			adjacentvertexindex[i*3*2+(2*k+1)]=vertexindex[j*3+(l+2)%3];
			adjacentvertexindex[j*3*2+(2*l+1)]=vertexindex[i*3+(k+2)%3];
			    
			j=NumVertices;
			break;
		    }
		}
	    }
	}
    }
    for(GLuint i=0;i<NumVertices*2;i++)
    {
	if(adjacentvertexindex[i]==(GLuint)-1)
	    std::cout<<i<<"th index not filled"<<std::endl;
	else if(adjacentvertexindex[i]>=NumVert)
	    std::cout<<i<<"th index pointing past end of vertex data"<<std::endl;
    }
	
	
    glGenVertexArrays(1,&VertexArrayObject);
    glBindVertexArray(VertexArrayObject);

    glGenBuffers(1,&VertexBuffer);
    glGenBuffers(1,&VertexIndexBuffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*NumVert,vertices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(vertices[0]),0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(vertices[0]),(GLvoid*)sizeof(vertices[0].Position));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,VertexIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLuint)*NumVertices*2,adjacentvertexindex,GL_STATIC_DRAW);
	
    delete [] vertexindex;
    delete [] vertices;
    delete [] adjacentvertexindex;
	
}
TriangleMesh::~TriangleMesh()
{
    glDeleteBuffers(1,&VertexBuffer);
    glDeleteBuffers(1,&VertexIndexBuffer);
    glDeleteVertexArrays(1,&VertexArrayObject);
}

void TriangleMesh::AddRef()
{
    RefCount++;
}

int TriangleMesh::RemoveRef()
{
    return --RefCount;
}

void TriangleMesh::Render()
{	
    glDrawElements(GL_TRIANGLES_ADJACENCY,NumVertices*2,GL_UNSIGNED_INT,(GLvoid*)0);
}

void TriangleMesh::SetMesh()
{
    glBindVertexArray(VertexArrayObject);
}

