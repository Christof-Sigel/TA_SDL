#pragma pack(push,1)

struct FILE_FNT_Char
{
    uint8_t Offset;
    uint8_t Segment;
};

struct FILE_FNT
{
    uint8_t Height;
    uint8_t Padding1[3];
    FILE_FNT_Char Character[255];
};

#pragma pack(pop)


struct FNTFont
{

};

void LoadCharacter(FILE_FNT_Char Character, uint8_t * FileBuffer, uint8_t * TextureBuffer, int CharNum, int Height, int TextureWidth)
{
    if(Character.Offset == 0 && Character.Segment ==0)
	return;
    int Width = * (FileBuffer + Character.Offset + Character.Segment * 0x100);
    if(Width == 0) 
	return;
    uint8_t * ImageData = (FileBuffer + Character.Offset + Character.Segment * 0x100 +1);
    int ByteOffset = 0, X =0, Y=0;
    while(Y< Height)
    {
	for(int i=0;i<8;i++)
	{
	    int bit = ImageData[ByteOffset]&(1<<i);
	    TextureBuffer[(CharNum*32+X + Y*TextureWidth)*4+0] = 0;
	    TextureBuffer[(CharNum*32+X + Y*TextureWidth)*4+1] = 0;
	    TextureBuffer[(CharNum*32+X + Y*TextureWidth)*4+2] = 0;
	    TextureBuffer[(CharNum*32+X + Y*TextureWidth)*4+3] = bit*255;
	    X++;
	    if(X>=Width)
	    {
		X=0;
		Y++;
		if(Y>=Height)
		    break;
	    }
	}
	ByteOffset++;
    }
}


void LoadFNTFont(uint8_t * Buffer, FNTFont * Font, int Size, GameState * CurrentGameState)
{
    uint8_t * ImageData = Buffer + sizeof(FILE_FNT);
    FILE_FNT * Header = (FILE_FNT*)Buffer;
    GLuint FontTexture;
    int SizeInColors = Size ;
    int TextureHeight =  Header->Height;
    int TextureWidth = 8096;
    STACK_ARRAY(TextureData, TextureWidth*TextureHeight*4, uint8_t);
    for(int i=0;i<250;i++)
    {
	LoadCharacter(Header->Character[i],Buffer, TextureData, i, Header->Height, TextureWidth);
    }
    glGenTextures(1,&FontTexture);
    glBindTexture(GL_TEXTURE_2D,FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TextureWidth,TextureHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);

}
