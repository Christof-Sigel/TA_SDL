#include "Gaf.hpp"
#include <string>
#include <iostream>
#include <string.h>

uint8_t TAPalette[] = 
		{0x00, 0x00, 0x00, 
		 0x80, 0x00, 0x00, 
		 0x00, 0x80, 0x00, 
		 0x80, 0x80, 0x00, 
		 0x00, 0x00, 0x80, 
		 0x80, 0x00, 0x80, 
		 0x00, 0x80, 0x80, 
		 0x80, 0x80, 0x80, 
		 0xC0, 0xDC, 0xC0, 
		 0x54, 0x54, 0xFC, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0xFF, 0xEB, 0xF3, 
		 0xEB, 0xC7, 0xD3, 
		 0xD7, 0xA3, 0xB3, 
		 0xC3, 0x87, 0x97, 
		 0xAF, 0x6F, 0x7F, 
		 0x9B, 0x5B, 0x63, 
		 0x8B, 0x47, 0x4F, 
		 0x7B, 0x3B, 0x47, 
		 0x6F, 0x33, 0x3B, 
		 0x63, 0x2B, 0x33, 
		 0x57, 0x23, 0x2B, 
		 0x4B, 0x1B, 0x27, 
		 0x3B, 0x17, 0x1F, 
		 0x2F, 0x0F, 0x17, 
		 0x23, 0x0B, 0x0F, 
		 0x17, 0x07, 0x0B, 
		 0x73, 0xFF, 0xDF, 
		 0x57, 0xE7, 0xBF, 
		 0x43, 0xCF, 0x9F, 
		 0x2F, 0xB7, 0x83, 
		 0x1F, 0x9F, 0x67, 
		 0x13, 0x8B, 0x4F, 
		 0x0F, 0x77, 0x3F, 
		 0x0B, 0x6B, 0x37, 
		 0x07, 0x5F, 0x2F, 
		 0x07, 0x53, 0x2B, 
		 0x00, 0x47, 0x27, 
		 0x00, 0x3F, 0x23, 
		 0x00, 0x33, 0x1B, 
		 0x00, 0x27, 0x17, 
		 0x00, 0x1B, 0x0F, 
		 0x00, 0x13, 0x0B, 
		 0xE3, 0xEF, 0xFF, 
		 0xC7, 0xDF, 0xE7, 
		 0xAF, 0xCF, 0xCB, 
		 0x93, 0xB7, 0xA7, 
		 0x7F, 0x9F, 0x83, 
		 0x6B, 0x87, 0x67, 
		 0x5F, 0x6F, 0x53, 
		 0x5F, 0x63, 0x47, 
		 0x5B, 0x57, 0x3B, 
		 0x53, 0x43, 0x33, 
		 0x47, 0x3B, 0x2B, 
		 0x3B, 0x33, 0x23, 
		 0x2F, 0x2B, 0x1B, 
		 0x23, 0x1F, 0x13, 
		 0x17, 0x13, 0x0F, 
		 0x0B, 0x0B, 0x07, 
		 0xFB, 0xFB, 0xD7, 
		 0xDF, 0xDF, 0xB7, 
		 0xC3, 0xC3, 0x9B, 
		 0xAB, 0xAB, 0x83, 
		 0x93, 0x93, 0x6F, 
		 0x77, 0x77, 0x57, 
		 0x63, 0x63, 0x43, 
		 0x53, 0x53, 0x33, 
		 0x43, 0x43, 0x23, 
		 0x33, 0x33, 0x17, 
		 0x23, 0x23, 0x0F, 
		 0x1B, 0x1B, 0x07, 
		 0x17, 0x17, 0x07, 
		 0x13, 0x13, 0x00, 
		 0x0F, 0x0F, 0x00, 
		 0x0B, 0x0B, 0x00, 
		 0xFB, 0xFB, 0xFB, 
		 0xEB, 0xEB, 0xEB, 
		 0xDB, 0xDB, 0xDB, 
		 0xCB, 0xCB, 0xCB, 
		 0xBB, 0xBB, 0xBB, 
		 0xAB, 0xAB, 0xAB, 
		 0x9B, 0x9B, 0x9B, 
		 0x8B, 0x8B, 0x8B, 
		 0x7B, 0x7B, 0x7B, 
		 0x6B, 0x6B, 0x6B, 
		 0x5B, 0x5B, 0x5B, 
		 0x4B, 0x4B, 0x4B, 
		 0x3B, 0x3B, 0x3B, 
		 0x2B, 0x2B, 0x2B, 
		 0x1F, 0x1F, 0x1F, 
		 0x0F, 0x0F, 0x0F, 
		 0xEB, 0xF3, 0xFF, 
		 0xCB, 0xE3, 0xFF, 
		 0xAF, 0xCF, 0xFF, 
		 0x97, 0xB3, 0xFF, 
		 0x7B, 0x97, 0xFF, 
		 0x67, 0x7F, 0xFF, 
		 0x53, 0x6B, 0xEF, 
		 0x3F, 0x5B, 0xE3, 
		 0x33, 0x4B, 0xD7, 
		 0x23, 0x3B, 0xCB, 
		 0x17, 0x2F, 0xAF, 
		 0x0F, 0x27, 0x97, 
		 0x07, 0x1F, 0x7B, 
		 0x07, 0x17, 0x63, 
		 0x00, 0x0F, 0x47, 
		 0x00, 0x0B, 0x2F, 
		 0xE3, 0xF7, 0xFF, 
		 0xBF, 0xDB, 0xE7, 
		 0x9F, 0xBF, 0xCF, 
		 0x83, 0xA7, 0xB7, 
		 0x6B, 0x8F, 0xA3, 
		 0x53, 0x77, 0x8B, 
		 0x3F, 0x5F, 0x73, 
		 0x2F, 0x4B, 0x5F, 
		 0x27, 0x3F, 0x57, 
		 0x23, 0x37, 0x4F, 
		 0x1F, 0x2F, 0x47, 
		 0x1B, 0x27, 0x3F, 
		 0x17, 0x1F, 0x37, 
		 0x13, 0x1B, 0x2F, 
		 0x0F, 0x13, 0x27, 
		 0x0B, 0x0F, 0x1F, 
		 0xD7, 0xEF, 0xFF, 
		 0xBB, 0xE3, 0xEF, 
		 0x9B, 0xCB, 0xDF, 
		 0x83, 0xB7, 0xCF, 
		 0x6B, 0xA3, 0xC3, 
		 0x53, 0x8F, 0xB3, 
		 0x3F, 0x7B, 0xA3, 
		 0x2F, 0x6B, 0x97, 
		 0x23, 0x5B, 0x87, 
		 0x1B, 0x4B, 0x77, 
		 0x13, 0x3F, 0x67, 
		 0x0B, 0x33, 0x57, 
		 0x07, 0x27, 0x47, 
		 0x00, 0x1B, 0x37, 
		 0x00, 0x13, 0x27, 
		 0x00, 0x0B, 0x1B, 
		 0xFF, 0xE7, 0xFF, 
		 0xE7, 0xC7, 0xEB, 
		 0xD3, 0xAB, 0xD7, 
		 0xBB, 0x93, 0xC3, 
		 0xA7, 0x7B, 0xB3, 
		 0x8F, 0x63, 0x9F, 
		 0x77, 0x4B, 0x8F, 
		 0x63, 0x3B, 0x7F, 
		 0x4F, 0x2B, 0x6F, 
		 0x43, 0x1F, 0x63, 
		 0x37, 0x17, 0x57, 
		 0x2B, 0x0F, 0x47, 
		 0x1F, 0x07, 0x3B, 
		 0x13, 0x00, 0x2B, 
		 0x0B, 0x00, 0x1F, 
		 0x07, 0x00, 0x13, 
		 0xD7, 0xFF, 0xA7, 
		 0xAB, 0xE7, 0x7F, 
		 0x83, 0xD3, 0x5B, 
		 0x67, 0xBF, 0x3F, 
		 0x4B, 0xAB, 0x2B, 
		 0x43, 0x97, 0x2B, 
		 0x37, 0x87, 0x27, 
		 0x2F, 0x77, 0x1B, 
		 0x2B, 0x67, 0x13, 
		 0x23, 0x5B, 0x0F, 
		 0x1F, 0x4F, 0x0B, 
		 0x1B, 0x43, 0x07, 
		 0x17, 0x33, 0x00, 
		 0x0F, 0x27, 0x00, 
		 0x0B, 0x1B, 0x00, 
		 0x07, 0x0F, 0x00, 
		 0xFF, 0xE3, 0x9F, 
		 0xE3, 0xC7, 0x73, 
		 0xCB, 0xAF, 0x53, 
		 0xB3, 0x97, 0x3F, 
		 0x9B, 0x83, 0x2F, 
		 0x83, 0x6F, 0x23, 
		 0x6B, 0x5B, 0x17, 
		 0x53, 0x47, 0x0F, 
		 0x4B, 0x3B, 0x0B, 
		 0x43, 0x33, 0x07, 
		 0x3B, 0x2B, 0x07, 
		 0x37, 0x23, 0x00, 
		 0x2F, 0x1B, 0x00, 
		 0x27, 0x13, 0x00, 
		 0x1F, 0x0F, 0x00, 
		 0x1B, 0x0B, 0x00, 
		 0xFF, 0xFF, 0xA3, 
		 0xFB, 0xF3, 0x83, 
		 0xF7, 0xE3, 0x67, 
		 0xF3, 0xD3, 0x4F, 
		 0xEF, 0xBB, 0x33, 
		 0xEF, 0xA7, 0x1B, 
		 0xEB, 0x8F, 0x13, 
		 0xE7, 0x7B, 0x0F, 
		 0xDF, 0x4F, 0x07, 
		 0xD7, 0x23, 0x00, 
		 0xBF, 0x1F, 0x00, 
		 0xA7, 0x1B, 0x00, 
		 0x93, 0x17, 0x00, 
		 0x7B, 0x13, 0x00, 
		 0x63, 0x13, 0x00, 
		 0x4F, 0x0F, 0x00, 
		 0xFF, 0xFF, 0x00, 
		 0xFF, 0xBF, 0x00, 
		 0xFF, 0x83, 0x00, 
		 0xFF, 0x47, 0x00, 
		 0xD3, 0x2B, 0x00, 
		 0xAB, 0x17, 0x00, 
		 0x7F, 0x07, 0x00, 
		 0x57, 0x00, 0x00, 
		 0xDF, 0xCB, 0xFF, 
		 0xBB, 0x9F, 0xDF, 
		 0x9B, 0x77, 0xBF, 
		 0x7F, 0x57, 0x9F, 
		 0x67, 0x3B, 0x7F, 
		 0x4B, 0x23, 0x5F, 
		 0x33, 0x13, 0x3F, 
		 0x1B, 0x07, 0x1F, 
		 0xD3, 0xDB, 0xFF, 
		 0x87, 0x9F, 0xF7, 
		 0x43, 0x6F, 0xEF, 
		 0x17, 0x47, 0xE7, 
		 0x0B, 0x2B, 0xBB, 
		 0x07, 0x17, 0x8F, 
		 0x00, 0x07, 0x63, 
		 0x00, 0x00, 0x37, 
		 0x7B, 0xFF, 0x77, 
		 0x53, 0xDF, 0x4F, 
		 0x33, 0xBF, 0x2B, 
		 0x1B, 0x9F, 0x13, 
		 0x1B, 0x7F, 0x0B, 
		 0x17, 0x5F, 0x07, 
		 0x13, 0x3F, 0x00, 
		 0x0B, 0x1F, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0x00, 0x00, 0x00, 
		 0xFF, 0xFB, 0xF0, 
		 0xA0, 0xA0, 0xA4, 
		 0x80, 0x80, 0x80, 
		 0xFF, 0x00, 0x00, 
		 0x00, 0xFF, 0x00, 
		 0xFF, 0xFF, 0x00, 
		 0x00, 0x00, 0xFF, 
		 0xFF, 0x00, 0xFF, 
		 0x00, 0xFF, 0xFF, 
		 0xFF, 0xFF, 0xFF};

struct Frame
{
    int16_t Width;
    int16_t Height;
    int16_t XOffset,YOffset;
    GLuint Texture;
};

struct GafEntry
{
    int16_t NumberOfFrames;
    std::string Name;
    Frame * Frames;
};

uint8_t * LoadFrameData(int offset,uint8_t * buffer, Frame * frame );

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
	//std::cout<<"Found Entry: "<<Entries[EntryIndex].Name<<" with "<<Entries[EntryIndex].NumberOfFrames<<" Frames"<<std::endl;
	Frame * Frames=Entries[EntryIndex].Frames;
	uint32_t SparseFramePointers[Entries[EntryIndex].NumberOfFrames*2];
	memcpy(SparseFramePointers,&buffer[offset],sizeof(uint32_t)*Entries[EntryIndex].NumberOfFrames*2);
	offset+=sizeof(uint32_t)*Entries[EntryIndex].NumberOfFrames*2;
	for(int FrameIndex=0;FrameIndex<Entries[EntryIndex].NumberOfFrames;FrameIndex++)
	{
	    offset=SparseFramePointers[FrameIndex*2];//the file format stores data_pointer, unknown long so we are skipping unkown here
	    uint8_t * FrameData=LoadFrameData(offset,buffer,&Frames[FrameIndex]);

	    if(FrameData!=nullptr)
	    {
		glGenTextures(1,&Frames[FrameIndex].Texture);
		glBindTexture(GL_TEXTURE_2D,Frames[FrameIndex].Texture);

		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,Frames[FrameIndex].Width,Frames[FrameIndex].Height,0, GL_RGBA, GL_UNSIGNED_BYTE, FrameData);
		delete [] FrameData;
	    }
	}
    }
}

GLuint Gaf::GetGLTexture(std::string Name)
{
    for(int EntryIndex=0;EntryIndex<NumEntries;EntryIndex++)
    {
	if(Entries[EntryIndex].Name.compare(Name)==0)
	    return Entries[EntryIndex].Frames[0].Texture;//return first frame for now
    }
    return 0;
}

uint8_t * LoadFrameData(int offset,uint8_t * buffer, Frame * frame )
{
    
    frame->Width=*(int16_t*)&buffer[offset];
    offset+=2;
    frame->Height=*(int16_t*)&buffer[offset];
    offset+=2;

    frame->XOffset=*(int16_t*)&buffer[offset];
    offset+=2;
    frame->YOffset=*(int16_t*)&buffer[offset];
    offset+=2;

    offset++;//unknown - apparently always 9
    int CompressionType=buffer[offset++];
	    
    int16_t NumSubFrames=*(int16_t*)&buffer[offset];
    offset+=2;

    //unknown data - apparently always 0
    offset+=4;

    int32_t FrameDataOffset=*(int32_t*)&buffer[offset];//either pixel data, if NumSubFrames==0 or More Entries otherwise
    int datasize=frame->Width * frame->Height*4;//three color + 1 alpha channel
    uint8_t * data=new uint8_t[datasize];
    for(int colorIndex=0;colorIndex<datasize;colorIndex+=4)
    {
	//set to (opaque) horrible #FF00FF pink
	data[colorIndex]=255;
	data[colorIndex+1]=0;
	data[colorIndex+2]=255;
	data[colorIndex+3]=255;
    }
    if(NumSubFrames==0)
    {
	if(CompressionType!=0)
	{
	    offset=FrameDataOffset;
	    for(int y=0;y<frame->Height;y++)
	    {
		int16_t linelength=*(int16_t*)&buffer[offset];
		offset+=2;
		int destoff=y*frame->Width*4;
		int nextline=offset+linelength;
		while(offset<nextline)
		{
		    uint8_t mask=buffer[offset++];
		    linelength--;
		    if(mask & 0x01)
		    {
			int skip=(mask>>1);
			for(int i=0;i<skip;i++)
			    data[destoff+i*4+3]=0;//transparent
			destoff+=skip*4;
		    }
		    else if(mask & 0x02)
		    {
			int repeat=(mask>>2)+1;
			for(int i=0;i<repeat;i++)
			{
			    for(int j=0;j<3;j++)
				data[destoff+i*4+j]=TAPalette[buffer[offset]*3+j];
			    data[destoff+i*4+3]=255;///opaque
			}
			destoff+=repeat*4;
			offset++;
				    }
		    else
		    {
			int numToCopy=(mask >> 2)+1;
			for(int i=0;i<numToCopy;i++)
			{
			    for(int j=0;j<3;j++)
				data[destoff+j]=TAPalette[buffer[offset]*3+j];
			    data[destoff+3]=255;//opaque
			    offset++;
			    destoff+=4;
			}
		    }
		}
	    }
	}
	else
	{
	    int size=frame->Width * frame->Height;
	    for(int i=0;i<size;i++)
	    {
		for(int j=0;j<3;j++)
		        data[i*4+j]=TAPalette[buffer[FrameDataOffset+i]*3+j];
		data[i*4+3]=255;//opaque
	    }
	}
    }
    else
    {
	std::cout<<"Not handling entry with "<<NumSubFrames<<" sub frames"<<std::endl;
    }
    return data;
}