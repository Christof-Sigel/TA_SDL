

#pragma pack(push,1)

struct FILE_TNTHeader
{
    uint32_t IDVersion;
    uint32_t Width;
    uint32_t Height;
    uint32_t MapDataOffset;
    uint32_t MapAttributeOffset;
    uint32_t TileGraphicsOffset;
    uint32_t NumberOfTiles;
    uint32_t NumberOfFeatures;
    uint32_t FeaturesOffset;
    uint32_t SeaLevel;
    uint32_t MiniMapOffset;
    uint32_t Unknown1;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
    uint32_t pad4;
};

struct FILE_TNTAttribute
{
    uint8_t Height;
    uint16_t FeatureIndex;
    uint8_t pad;//probably padding, not entirely clear
};

struct FILE_TNTTile
{
    uint8_t TileData[32*32];
};

struct FILE_TNTFeature
{
    uint32_t Index;//this apparently matches the index used to get to this entry?
    char name[128];
};

#pragma pack(pop)



struct TAMap
{
    GLuint MapVertexBuffer, MapTexture, MinimapTexture;
    int NumTriangles;
    void Render(ShaderProgram * MapShader)
    {
	glUseProgram(MapShader->ProgramID);
	glBindTexture(GL_TEXTURE_2D,MapTexture);
	glBindVertexArray(MapVertexBuffer);
	glDrawArrays(GL_TRIANGLES, 0, NumTriangles*3);
	
    }

    void RenderMiniMap()
    {
	//TODO(Christof): Update UIElements to have background (will use this here)
    }
};


const int TNT_HEADER_ID=0x2000;
float GetTileXTex(int TileIndex, float Offset, int TextureSide)
{
    int X = TileIndex % (TextureSide/32);
    return (X + Offset)/(TextureSide/32);
}

float GetTileYTex(int TileIndex, float Offset, int TextureSide)
{
    int Y = TileIndex / (TextureSide/32);
    return (Y + Offset)/(TextureSide/32);
}

float GetHeightFor(int X, int Y, FILE_TNTAttribute * Attributes, int Width,int Height)
{
    const float HEIGHT_MOD = 1/255.0;
    float result=0;
    for(int i=0;i<2;i++)
    {
	for(int j=0;j<2;j++)
	{
	    int x=X+i;
	    int y=Y+j;
	    if(x<0)
		x=0;
	    if(y<0)
		y=0;
	    if(y>=Height)
		y=Height-1;
	    if(x>=Width)
		x=Width-1;
	    result+=Attributes[x+y*Width].Height*HEIGHT_MOD;
	}
    }
    return result;
}

bool32 LoadTNTFromBuffer(char * Buffer, TAMap * Result)
{
    FILE_TNTHeader * Header = (FILE_TNTHeader *)Buffer;
    if(Header->IDVersion != TNT_HEADER_ID)
    {
	return 0;
    }
    LogDebug("%dx%d",Header->Width,Header->Height);
    uint16_t * TileIndices = (uint16_t*)(Buffer + Header->MapDataOffset);//these map to 32x32 tiles NOT 16x16 half tiles
    int NumberOfHalfTiles = Header->Width*Header->Height;
    int NumberOfTiles = NumberOfHalfTiles /4 ;//heights and features defined on 16x16 tiles, graphics tiles are 32x32
    FILE_TNTAttribute * Attributes=(FILE_TNTAttribute *)(Buffer + Header->MapAttributeOffset);//half tiles
    FILE_TNTTile * Tiles=(FILE_TNTTile *)(Buffer + Header->TileGraphicsOffset);
    FILE_TNTFeature * Features=(FILE_TNTFeature*)(Buffer + Header->FeaturesOffset);

    int MinimapWidth, MinimapHeight;
    MinimapWidth = *(uint32_t*)(Buffer + Header->MiniMapOffset);
    MinimapHeight = *(uint32_t*)(Buffer + Header->MiniMapOffset+4);
    uint8_t * MinimapData = (uint8_t *)(Buffer + Header->MiniMapOffset + 8);

    //TODO(Christof): TA Apparently uses 0xDD to denote transparency, need to deal with this here

    int TileTextureSide = ceil(sqrt(Header->NumberOfTiles *32*32));
    TileTextureSide += 32-(TileTextureSide%32);
    uint8_t * TileTextureData = (uint8_t*)malloc(TileTextureSide*TileTextureSide *4);
    memset(TileTextureData, 0, TileTextureSide*TileTextureSide*4);
    int i=0;

    for(int Y=0;Y<TileTextureSide;Y+=32)
    {
	for(int X=0;X<TileTextureSide;X+=32)
	{
	    for(int y=0;y<32;y++)
	    {
		for(int x=0;x<32;x++)
		{
		    if(Tiles[i].TileData[x+y*32]=='d')
		    {
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+0]=255;
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+1]=0;
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+2]=255;
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+3]=0;//PaletteData[Tiles[i].TileData[x+y*32]*4+0];
		    }
		    else
		    {
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+0]=PaletteData[Tiles[i].TileData[x+y*32]*4+0];
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+1]=PaletteData[Tiles[i].TileData[x+y*32]*4+1];
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+2]=PaletteData[Tiles[i].TileData[x+y*32]*4+2];
			TileTextureData[(X+x+(Y+y)*TileTextureSide)*4+3]=255;//PaletteData[Tiles[i].TileData[x+y*32]*4+0];
		    }
		    
		    // TileTextureData[(x+i*32+y*Header->NumberOfTiles*32)*4+1]=PaletteData[Tiles[i].TileData[x+y*32]*4+1];
		    // TileTextureData[(x+i*32+y*Header->NumberOfTiles*32)*4+2]=PaletteData[Tiles[i].TileData[x+y*32]*4+2];
		    // TileTextureData[(x+i*32+y*Header->NumberOfTiles*32)*4+3]=255;//PaletteData[Tiles[i].TileData[x+y*32]*4+3];
		}
	    }
	    i++;
	    if(i>=Header->NumberOfTiles)
		goto texture_done;
	}
    }
    

texture_done:
    glGenTextures(1,&Result->MapTexture);
    glBindTexture(GL_TEXTURE_2D,Result->MapTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TileTextureSide, TileTextureSide,0, GL_RGBA, GL_UNSIGNED_BYTE, TileTextureData);
    free(TileTextureData);


    
    const int NUM_FLOATS_PER_HALFTILE=2*3*(3+2);//2 triangles per half tile, 3 verts per tri, 3 poscoords + 2 texcoords per vert
    GLfloat * PositionAndTexture = (GLfloat *)malloc(sizeof(GLfloat)*NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE);

    //TODO(Christof): Store heightmap at least for collision stuff later?
    for(int X=0;X<Header->Width;X++)
    {
	for(int Y=0;Y<Header->Height;Y++)
	{
	    float Height=GetHeightFor(X-1,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+0]=X/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+1]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+2]=Y/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+3]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+4]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0,TileTextureSide);

	    Height=GetHeightFor(X,Y,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+5]=(X+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+6]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+7]=(Y+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+8]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0:0.5,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+9]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0:0.5,TileTextureSide);

	    Height=GetHeightFor(X,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+10]=(X+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+11]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+12]=Y/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+13]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0:0.5,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+14]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0,TileTextureSide);

	    Height=GetHeightFor(X-1,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+15]=X/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+16]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+17]=Y/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+18]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+19]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0,TileTextureSide);

	    Height=GetHeightFor(X-1,Y,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+20]=X/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+21]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+22]=(Y+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+23]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+24]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0:0.5,TileTextureSide);

	    Height=GetHeightFor(X,Y,Attributes,Header->Width,Header->Height);
	     PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+25]=(X+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+26]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+27]=(Y+1)/10.0;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+28]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0:0.5,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+29]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0:0.5,TileTextureSide);
	   
	    
	}
    }


    glGenVertexArrays(1,&Result->MapVertexBuffer);
    glBindVertexArray(Result->MapVertexBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,PositionAndTexture,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*NUM_FLOATS_PER_HALFTILE/6,0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*NUM_FLOATS_PER_HALFTILE/6,(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    // glDeleteBuffers(1,&VertexBuffer);
    Result->NumTriangles = NumberOfHalfTiles*2;
    free(PositionAndTexture);

    return 1;
}
