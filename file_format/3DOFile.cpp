#include "3DOFile.hpp"
#include <iostream>
#include <string.h>

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

    int OffsetToSelectionPrimitive=*(int32_t*)(&buffer[offset]);
    offset+=4;

    int X=*(int32_t*)(&buffer[offset]);
    offset+=4;
    int Y=*(int32_t*)(&buffer[offset]);
    offset+=4;
    int Z=*(int32_t*)(&buffer[offset]);
    offset+=4;

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

    Children = new Unit3DObject *[NumChildren];
    for(int i=0;i<NumChildren;i++)
    {
	Children[i]=new Unit3DObject(buffer,ChildOffset);
	ChildOffset=*(int32_t *)&buffer[ChildOffset+OffsetToSibling];
    }

    int32_t Vertices[NumVertices*3];
    memcpy(Vertices,&buffer[VertexArrayOffset],NumVertices*4*3);//read 3 32-bit integers per vertex 
}
