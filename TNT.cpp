


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
    const float HEIGHT_MOD = 1/25.50;
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

bool32 LoadTNTFromBuffer(uint8_t * Buffer, TAMap * Result,uint8_t * PaletteData, MemoryArena * TempArena)
{
    FILE_TNTHeader * Header = (FILE_TNTHeader *)Buffer;
    if(Header->IDVersion != TNT_HEADER_ID)
    {
	return 0;
    }
    LogDebug("%dx%d",Header->Width,Header->Height);
    uint16_t * TileIndices = (uint16_t*)(Buffer + Header->MapDataOffset);//these map to 32x32 tiles NOT 16x16 half tiles
    int NumberOfHalfTiles = Header->Width*Header->Height;
    FILE_TNTAttribute * Attributes=(FILE_TNTAttribute *)(Buffer + Header->MapAttributeOffset);//half tiles
    FILE_TNTTile * Tiles=(FILE_TNTTile *)(Buffer + Header->TileGraphicsOffset);
    //TODO(Christof): Figure out how to deal with the features (upright billboard perhaps?) FILE_TNTFeature * Features=(FILE_TNTFeature*)(Buffer + Header->FeaturesOffset);

    int MinimapWidth, MinimapHeight;
    MinimapWidth = *(uint32_t*)(Buffer + Header->MiniMapOffset);
    MinimapHeight = *(uint32_t*)(Buffer + Header->MiniMapOffset+4);
    //Not using Minimap Data at the moment - silence some compiler warnings uint8_t * MinimapData = (uint8_t *)(Buffer + Header->MiniMapOffset + 8);

    //TODO(Christof): TA Apparently uses 0xDD to denote transparency, need to deal with this here

    int TileTextureSide = (int)ceil(sqrt(Header->NumberOfTiles *32*32));
    TileTextureSide += 32-(TileTextureSide%32);
    uint8_t * TileTextureData = PushArray(TempArena,TileTextureSide*TileTextureSide*4,uint8_t);
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
	    if((uint32_t)i>=Header->NumberOfTiles)
		goto texture_done;
	}
    }
    

texture_done:
    glGenTextures(1,&Result->MapTexture);
    glBindTexture(GL_TEXTURE_2D,Result->MapTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TileTextureSide, TileTextureSide,0, GL_RGBA, GL_UNSIGNED_BYTE, TileTextureData);
    PopArray(TempArena,TileTextureData,TileTextureSide*TileTextureSide*4,uint8_t);


    
    const int NUM_FLOATS_PER_HALFTILE=2*3*(3+2);//2 triangles per half tile, 3 verts per tri, 3 poscoords + 2 texcoords per vert
    GLfloat * PositionAndTexture = PushArray(TempArena,NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,GLfloat);

    float UnitsPerTile=6.0;
    //TODO(Christof): Store heightmap at least for collision stuff later?
    for(uint32_t X=0;X<Header->Width;X++)
    {
	for(uint32_t Y=0;Y<Header->Height;Y++)
	{
	    float Height=GetHeightFor(X-1,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+0]=X*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+1]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+2]=Y*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+3]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+4]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0f,TileTextureSide);

	    Height=GetHeightFor(X,Y,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+5]=(X+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+6]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+7]=(Y+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+8]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0f:0.5f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+9]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0f:0.5f,TileTextureSide);

	    Height=GetHeightFor(X,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+10]=(X+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+11]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+12]=Y*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+13]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0f:0.5f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+14]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0f,TileTextureSide);

	    Height=GetHeightFor(X-1,Y-1,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+15]=X*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+16]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+17]=Y*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+18]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+19]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2/2.0f,TileTextureSide);

	    Height=GetHeightFor(X-1,Y,Attributes,Header->Width,Header->Height);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+20]=X*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+21]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+22]=(Y+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+23]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2/2.0f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+24]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0f:0.5f,TileTextureSide);

	    Height=GetHeightFor(X,Y,Attributes,Header->Width,Header->Height);
	     PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+25]=(X+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+26]=Height;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+27]=(Y+1)*UnitsPerTile;
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+28]=GetTileXTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],X%2?1.0f:0.5f,TileTextureSide);
	    PositionAndTexture[X*NUM_FLOATS_PER_HALFTILE+Y*Header->Width*NUM_FLOATS_PER_HALFTILE+29]=GetTileYTex(TileIndices[X/2+(Y/2)*(Header->Width/2)],Y%2?1.0f:0.5f,TileTextureSide);
	   
	    
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
    PopArray(TempArena,PositionAndTexture,NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,GLfloat);

    return 1;
}
