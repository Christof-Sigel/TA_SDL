#include "3DOFile.hpp"
#include <iostream>
#include <string.h>
#include <cmath>


const float TA_TO_GL_SCALE=1/2500000.0f;//this gives an arm solar collector of approximately size 2x2 units
const int FLOATS_PER_TRIANGLE=15;//each triangle has 3x3 position coords, and 3x2 texture coords
const int COLORS_PER_TRIANGLE=3;
void FillArraysForTriangle(GLfloat * PosTexArray,uint16_t * Indexes, float * Vertices,float * UVCoords,int16_t ColorIndex,int16_t * ColorIndexes)
{
    for(int vertex=0;vertex<3;vertex++)
    {
	PosTexArray[vertex*5]=Vertices[Indexes[vertex]*3];
	PosTexArray[vertex*5+1]=Vertices[Indexes[vertex]*3+1];
	PosTexArray[vertex*5+2]=Vertices[Indexes[vertex]*3+2];
	PosTexArray[vertex*5+3]=UVCoords[vertex*3];
	PosTexArray[vertex*5+4]=UVCoords[vertex*3+1];
	ColorIndexes[vertex]=ColorIndex;
    }
}

Unit3DObject::Unit3DObject(unsigned char * buffer, int offset)
{
    if(*(int32_t*)&buffer[offset]!=1)
    {
	std::cout<<"Unknown signature "<<*(int32_t*)&buffer[offset]<<std::endl;
	return;
    }
    offset+=4;
    int NumVertices=*(int32_t*)(&buffer[offset]);
    offset+=4;

    int NumPrimitives=*(int32_t*)(&buffer[offset]);
    offset+=4;

    //this will apparently be -1 for all child objects - will need ignore in that case
    int OffsetToSelectionPrimitive=*(int32_t*)(&buffer[offset]);
    offset+=4;

    int X=*(int32_t*)(&buffer[offset]);
    offset+=4;
    int Y=*(int32_t*)(&buffer[offset]);
    offset+=4;
    int Z=*(int32_t*)(&buffer[offset]);
    offset+=4;
    GLfloat pos[]={X*TA_TO_GL_SCALE,Y*TA_TO_GL_SCALE,Z*TA_TO_GL_SCALE};
    LocationMatrix.SetPosition(pos);

    int NameOffset=*(int32_t*)(&buffer[offset]);
    Name=std::string((char *)&buffer[NameOffset]);
    offset+=4;
    //Dont care about apparently always 0 field
    //TODO: check field for all cavedog models in case some are not 0 and this field is significant
    offset+=4;

    int VertexArrayOffset=*(int32_t*)(&buffer[offset]);
    offset+=4;
    int PrimitiveArrayOffset=*(int32_t*)(&buffer[offset]);
    offset+=4;

    int OffsetToSibling=offset;
    offset+=4;
    int ChildOffset=*(int32_t*)(&buffer[offset]);

    NumChildren=0;
    if(ChildOffset!=0)
    {
	NumChildren++;
	int SiblingOffset=*(int32_t *)&buffer[ChildOffset+OffsetToSibling];
	while(SiblingOffset!=0)
	{
	    SiblingOffset=*(int32_t *)&buffer[SiblingOffset+OffsetToSibling];
	    NumChildren++;
	}
    }
   
    int32_t IntVertices[NumVertices*3];
    memcpy(IntVertices,&buffer[VertexArrayOffset],NumVertices*4*3);//read 3 32-bit integers per vertex
    GLfloat Vertices[NumVertices*3];
    for(int vertIndex=0;vertIndex<NumVertices*3;vertIndex++)
    {
	Vertices[vertIndex]=IntVertices[vertIndex]*TA_TO_GL_SCALE;
    }
    
    int32_t PossibleTextures[NumPrimitives];
    int PrimitiveToTextureMap[NumPrimitives];
    int CurrentTexNum=0;
    for(int PrimIndex=0;PrimIndex<NumPrimitives;PrimIndex++)
    {
	//docs were unclear on this, but data shows this is the primitive index, not a buffer offset
	if(OffsetToSelectionPrimitive == PrimIndex)
	{
	    //this is the selection primitive - just ignore for now, should load it into the
	    //selection VAO for rendering later
	    PrimitiveToTextureMap[PrimIndex]=-1;
	    continue;
	}
	int32_t TexOffset=*(int32_t*)&buffer[PrimitiveArrayOffset+16+PrimIndex*32];//texture offset is at the 5th 32-bit int, Primitive has 8 32-bit ints
	//Not ignoring 0 here as we need to group no texture primitives and render them separately from the others
	bool found=false;
	for(int TexIndex=0;TexIndex<CurrentTexNum;TexIndex++)
	{
	    if(PossibleTextures[TexIndex]==TexOffset)
	    {
		found=true;
		PrimitiveToTextureMap[PrimIndex]=TexIndex;
		break;
	    }
	}
	if(!found)
	{
	    PrimitiveToTextureMap[PrimIndex]=CurrentTexNum;
	    PossibleTextures[CurrentTexNum++]=TexOffset;
	}
    }
    NumTextures=CurrentTexNum;
    NumTriangles=new int[NumTextures];
    Textures=new GLuint[NumTextures];
    VertexArrayObjects=new GLuint[NumTextures];
    glGenVertexArrays(NumTextures,VertexArrayObjects);
    memset(Textures,0,sizeof(GLuint)*NumTextures);

    for(int TextureIndex=0; TextureIndex<NumTextures;TextureIndex++)
    {
	if(PossibleTextures[TextureIndex]!=0)
	{
	    //TODO: Load texture from global pool
	}
	NumTriangles[TextureIndex]=0;
	for(int PrimIndex=0;PrimIndex<NumPrimitives;PrimIndex++)
	{
	    if(PrimitiveToTextureMap[PrimIndex]==TextureIndex)
	    {
		int offset=PrimitiveArrayOffset+PrimIndex*32;
		offset+=4;

		int32_t NumberOfVertices=*(int32_t *)&buffer[offset];
		NumTriangles[TextureIndex]+=NumberOfVertices-2>0?NumberOfVertices-2:0;
	    }
	}
	GLfloat PositionAndTexCoord[NumTriangles[TextureIndex]*FLOATS_PER_TRIANGLE];
	int16_t ColorIndexes[NumTriangles[TextureIndex]*COLORS_PER_TRIANGLE];
	int CurrentTriangle=0;
	for(int PrimIndex=0;PrimIndex<NumPrimitives;PrimIndex++)
	{
	    if(PrimitiveToTextureMap[PrimIndex]==TextureIndex)
	    {
		int offset=PrimitiveArrayOffset+PrimIndex*32;
		int32_t ColorIndex=*(int32_t *)&buffer[offset];
		offset+=4;

		int32_t NumberOfVertices=*(int32_t *)&buffer[offset];
		offset+=4;

		offset+=4;//this field is apparently always 0, maybe test later?

		int32_t VertexIndexArrayOffset=*(int32_t *)&buffer[offset];
		offset+=4;

		offset+=4;//this is the texture offset, which we have already stored

		//the final three 32-bit ints are apparently cavedog specific and only used for their editor?
		uint16_t * IndexArray=(uint16_t*)&buffer[VertexIndexArrayOffset];

		switch(NumberOfVertices)
		{
		case 1:
		case 2:
		    //Ignore points and lines for rendering, at least for now
		    std::cout<<"Line or point ignored"<<std::endl;
		    break;
		case 3:
		{
		    GLfloat UVCoords[]={0,0,0,1,1,1};
		    FillArraysForTriangle(&PositionAndTexCoord[CurrentTriangle*FLOATS_PER_TRIANGLE],IndexArray,Vertices,UVCoords,ColorIndex,&ColorIndexes[CurrentTriangle*COLORS_PER_TRIANGLE]);
		    CurrentTriangle++;
		}
		    break;
		case 4:
		{
		    GLfloat UVCoords[]={0,0,1,0,1,1,0,0,1,1,0,1};
		    uint16_t TempIndexes[]={IndexArray[0],IndexArray[3],IndexArray[2]};
		    FillArraysForTriangle(&PositionAndTexCoord[CurrentTriangle*FLOATS_PER_TRIANGLE],TempIndexes,Vertices,&UVCoords[6],ColorIndex,&ColorIndexes[CurrentTriangle*COLORS_PER_TRIANGLE]);
		    TempIndexes[1]=IndexArray[2];
		    TempIndexes[2]=IndexArray[1];
		    CurrentTriangle++;
		    FillArraysForTriangle(&PositionAndTexCoord[CurrentTriangle*FLOATS_PER_TRIANGLE],TempIndexes,Vertices,UVCoords,ColorIndex,&ColorIndexes[CurrentTriangle*COLORS_PER_TRIANGLE]);
		    CurrentTriangle++;
		}
		    break;
		case 5:
		    std::cout<<"Currently not treating pentagons specially"<<std::endl;
		default:
		{
		    std::cout<<"Found a primitive of size "<<NumberOfVertices<<", using default 6+ side polygon code"<<std::endl;
		    for(int i=0;i<NumberOfVertices-2;i++)
		    {
			uint16_t TempIndexes[]={IndexArray[i],IndexArray[i+1],IndexArray[NumberOfVertices-1]};
			GLfloat UVCoords[6];
			for(int j=0;j<3;j++)
			{
			    UVCoords[j*2]=0.5f*(1-(sin((2.0f*(i+j)+1)*PI/float(NumberOfVertices))/cos(PI/NumberOfVertices)));
			    UVCoords[j*2+1]=0.5f*(1-(cos(PI/NumberOfVertices*(2.0f*(i+j)+1))/cos(PI/NumberOfVertices)));
			}
			FillArraysForTriangle(&PositionAndTexCoord[CurrentTriangle*FLOATS_PER_TRIANGLE],TempIndexes,Vertices,UVCoords,ColorIndex,&ColorIndexes[CurrentTriangle*COLORS_PER_TRIANGLE]);
			CurrentTriangle++;
		    }
		}
		    break;
		}
	    }
	}
	glBindVertexArray(VertexArrayObjects[TextureIndex]);

	GLuint VertexBuffer;
	glGenBuffers(1,&VertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*NumTriangles[TextureIndex]*FLOATS_PER_TRIANGLE,PositionAndTexCoord,GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*FLOATS_PER_TRIANGLE/3,0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*FLOATS_PER_TRIANGLE/3,(GLvoid*)(sizeof(GLfloat)*3));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint ColorBuffer;
	glGenBuffers(1,&ColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,ColorBuffer);
	glBufferData(GL_ARRAY_BUFFER,sizeof(int16_t)*NumTriangles[TextureIndex]*COLORS_PER_TRIANGLE,ColorIndexes,GL_STATIC_DRAW);
	glVertexAttribPointer(2,1,GL_SHORT,GL_FALSE,0,0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glDeleteBuffers(1,&VertexBuffer);
	glDeleteBuffers(1,&ColorBuffer);
    }

    Children = new Unit3DObject *[NumChildren];
    for(int i=0;i<NumChildren;i++)
    {
	Children[i]=new Unit3DObject(buffer,ChildOffset);
	ChildOffset=*(int32_t *)&buffer[ChildOffset+OffsetToSibling];
    }

}

void Unit3DObject::Render(Matrix Model,GLuint ModelViewLoc,Matrix ParentTrans)
{
    Matrix NewTrans=LocationMatrix*ParentTrans;
    (Model*NewTrans).Upload(ModelViewLoc);
    for(int TextureIndex=0;TextureIndex<NumTextures;TextureIndex++)
    {
	glBindVertexArray(VertexArrayObjects[TextureIndex]);
	glDrawArrays(GL_TRIANGLES,0,NumTriangles[TextureIndex]*3);
    }
    for(int ChildIndex=0;ChildIndex<NumChildren;ChildIndex++)
    {
	Children[ChildIndex]->Render(Model,ModelViewLoc,NewTrans);
    }
}
