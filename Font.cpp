#pragma pack(push,1)


struct FILE_FNT
{
    uint8_t Height;
    uint8_t Padding1[3];
    int16_t CharacterOffset[255];
};

#pragma pack(pop)


struct FNTFont
{

};

void LoadCharacter(int16_t CharacterOffset, uint8_t * FileBuffer, uint8_t * TextureBuffer, int XOffset, int YOffset, int Height, int TextureWidth)
{
    if(CharacterOffset ==0)
	return;
    int Width = * (FileBuffer + CharacterOffset );
    if(Width == 0) 
	return;
    uint8_t * ImageData = (FileBuffer + CharacterOffset + 1);
    int ByteOffset = 0, X =0, Y=0;
    while(Y< Height)
    {
	for(int i=7;i>=0;i--)
	{
	    int bit = ImageData[ByteOffset]&(1<<i);
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+0] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+1] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+2] = 0;
	    TextureBuffer[(XOffset+X + (Y+YOffset)*TextureWidth)*4+3] = bit*255;
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
    int TextureHeight =  Header->Height*32;
    int TextureWidth = 8096/16;
    STACK_ARRAY(TextureData, TextureWidth*TextureHeight*4, uint8_t);
    for(int i=0;i<254;i++)
    {
	LoadCharacter(Header->CharacterOffset[i] ,Buffer, TextureData, (i%32)*16, (i/32)*16, Header->Height, TextureWidth);
    }
    glGenTextures(1,&FontTexture);
    glBindTexture(GL_TEXTURE_2D,FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TextureWidth,TextureHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);

}
