#include "3DOFile.hpp"
#include <iostream>
#include <string.h>

const float TA_TO_GL_SCALE=1/10000000.0f;//this is the scaling I was using for the old project, may need to look into a better scale


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
    std::cout<<Name<<" ("<<X<<","<<Y<<","<<Z<<") ";
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
	std::cout<<" has "<<NumChildren<<" children";
    }
    std::cout<<std::endl;
    int32_t Vertices[NumVertices*3];
    memcpy(Vertices,&buffer[VertexArrayOffset],NumVertices*4*3);//read 3 32-bit integers per vertex
    
    int32_t PossibleTextures[NumPrimitives];
    int CurrentTexNum=0;
    for(int PrimIndex=0;PrimIndex<NumPrimitives;PrimIndex++)
    {
	int32_t TexOffset=*(int32_t*)&buffer[PrimitiveArrayOffset+16+PrimIndex*32];//texture offset is at the 5th 32-bit int, Primitive has 8 32-bit ints
	//Not ignoring 0 here as we need to group no texture primitives and render them separately from the others
	bool found=false;
	for(int TexIndex=0;TexIndex<CurrentTexNum;TexIndex++)
	{
	    if(PossibleTextures[TexIndex]==TexOffset)
	    {
		found=true;
		break;
	    }
	}
	if(!found)
	{
	    PossibleTextures[CurrentTexNum++]=TexOffset;
	}
    }

    NumTextures=CurrentTexNum;

     

    Children = new Unit3DObject *[NumChildren];
    for(int i=0;i<NumChildren;i++)
    {
	Children[i]=new Unit3DObject(buffer,ChildOffset);
	ChildOffset=*(int32_t *)&buffer[ChildOffset+OffsetToSibling];
    }

}
