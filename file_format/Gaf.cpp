#include "Gaf.hpp"
#include <string>
#include <iostream>
#include <string.h>


struct Frame
{
    int16_t Width;
    int16_t Height;
    int16_t XOffset,YOffset;
    uint8_t * FrameData;
};

struct GafEntry
{
    int16_t NumberOfFrames;
    std::string Name;
    Frame * Frames;
};


Gaf::Gaf(unsigned char * buffer)
{
    int offset=0;
    int32_t VersionID=*(int32_t*)&buffer[offset];
    offset+=4;
    if(VersionID!=0x00010100)//Version stamp
    {
	std::cout<<"Unknown gaf version stamp: "<<VersionID<<std::endl;
	return;
    }

    NumEntries=*(int32_t *)&buffer[offset];
    offset+=4;
    offset+=4;//don't care about unknown (apparently always 0) field

    int32_t EntryPointers[NumEntries];
    Entries=new GafEntry[NumEntries];
    memcpy(EntryPointers,&buffer[offset],sizeof(int32_t)*NumEntries);

    for(int EntryIndex=0;EntryIndex<NumEntries;EntryIndex++)
    {
	offset=EntryPointers[EntryIndex];
	Entries[EntryIndex].NumberOfFrames=*(int16_t*)&buffer[offset];
	offset+=2;
	offset+=2;//unknown, apparently always 1
	offset+=4;//unknown, apparently always 0

	char NameChar[33];
	memset(NameChar,0,33);
	memcpy(NameChar,&buffer[offset],32);
	offset+=32;
	Entries[EntryIndex].Name=std::string(NameChar);
	Entries[EntryIndex].Frames=new Frame[Entries[EntryIndex].NumberOfFrames];
	std::cout<<"Found Entry: "<<Entries[EntryIndex].Name<<" with "<<Entries[EntryIndex].NumberOfFrames<<" Frames"<<std::endl;
	Frame * Frames=Entries[EntryIndex].Frames;
	uint32_t SparseFramePointers[Entries[EntryIndex].NumberOfFrames*2];
	memcpy(SparseFramePointers,&buffer[offset],sizeof(uint32_t)*Entries[EntryIndex].NumberOfFrames*2);
	offset+=sizeof(uint32_t)*Entries[EntryIndex].NumberOfFrames*2;
	for(int FrameIndex=0;FrameIndex<Entries[EntryIndex].NumberOfFrames;FrameIndex++)
	{
	    offset=SparseFramePointers[FrameIndex*2];//the file format stores data_pointer, unknown long so we are skipping unkown here
	    Frames[FrameIndex].Width=*(int16_t*)&buffer[offset];
	    offset+=2;
	    Frames[FrameIndex].Height=*(int16_t*)&buffer[offset];
	    offset+=2;

	    Frames[FrameIndex].XOffset=*(int16_t*)&buffer[offset];
	    offset+=2;
	    Frames[FrameIndex].YOffset=*(int16_t*)&buffer[offset];
	    offset+=2;

	    offset++;//unknown - apparently always 9
	    int CompressionType=buffer[offset++];
	    
	    int16_t NumSubFrames=*(int16_t*)&buffer[offset];
	    offset+=2;

	    int32_t FrameDataOffset=*(int32_t*)&buffer[offset];//either pixel data, if NumSubFrames==0 or More Entries otherwise
	    if(NumSubFrames==0)
	    {

	    }
	    else
	    {
		std::cout<<"Not handling entry with "<<NumSubFrames<<" sub frames"<<std::endl;
	    }
	}
    }
}

GLuint Gaf::GetGLTexture(std::string Name)
{
    
}
