internal inline float GetTileXTex(int TileIndex, float Offset, int TextureSide)
{
    int X = TileIndex % (TextureSide/32);
    return (X + Offset)/(TextureSide/32);
}

internal inline float GetTileYTex(int TileIndex, float Offset, int TextureSide)
{
    int Y = TileIndex / (TextureSide/32);
    return (Y + Offset)/(TextureSide/32);
}

internal inline float GetHeightFor(u32 X, u32 Y, FILE_TNTAttribute * Attributes, u32 Width,u32 Height)
{
    const float HEIGHT_MOD = 1/25.50f;
    float result=0;
    for(u32 i=0;i<2;i++)
    {
	for(u32 j=0;j<2;j++)
	{
	    u32 x=X+i;
	    u32 y=Y+j;
	    if(y>=Height)
		y=Height-1;
	    if(x>=Width)
		x=Width-1;
	    result+=Attributes[x+y*Width].Height*HEIGHT_MOD;
	}
    }
    return result;
}

void RenderMap(TAMap * Map, ShaderProgram * MapShader)
{
    glUseProgram(MapShader->ProgramID);
    glBindTexture(GL_TEXTURE_2D,Map->MapTexture);
    glBindVertexArray(Map->MapVertexBuffer);
    glDrawArrays(GL_TRIANGLES, 0, Map->NumTriangles*3);
}

void DrawMiniMap(TAMap * Map, r32 X, r32 Y, r32 Width, r32 Height, Color Color, r32 Alpha, Texture2DShaderDetails * ShaderDetails)
{
    DrawTexture2D(Map->MinimapTexture, X, Y, Width, Height, Color, Alpha,  ShaderDetails,0,0, 1,1);
}


internal b32 LoadTNTFromBuffer(u8 * Buffer, TAMap * Result,u8 * PaletteData, MemoryArena * TempArena)
{
    FILE_TNTHeader * Header = (FILE_TNTHeader *)Buffer;
    if(Header->IDVersion != TNT_HEADER_ID)
    {
	return 0;
    }
    u16  * TileIndices = (u16 *)(Buffer + Header->MapDataOffset);//these map to 32x32 tiles NOT 16x16 half tiles
    u32 NumberOfHalfTiles = Header->Width*Header->Height;
    FILE_TNTAttribute * Attributes=(FILE_TNTAttribute *)(Buffer + Header->MapAttributeOffset);//half tiles
    FILE_TNTTile * Tiles=(FILE_TNTTile *)(Buffer + Header->TileGraphicsOffset);
    //TODO(Christof): Figure out how to deal with the features (upright billboard perhaps?) FILE_TNTFeature * Features=(FILE_TNTFeature*)(Buffer + Header->FeaturesOffset);


    Result->MinimapWidth = (s32)*(u32 *)(Buffer + Header->MiniMapOffset);
    Result->MinimapHeight = (s32)*(u32 *)(Buffer + Header->MiniMapOffset+4);


    u8 * MinimapData = (u8 *)(Buffer + Header->MiniMapOffset + 8);
    u8 * MinimapTexture = PushArray(TempArena, Result->MinimapWidth *  Result->MinimapHeight * 4, u8);
    for(s32 y=0;y < Result->MinimapHeight;y++)
    {
	for(s32 x = 0;x<Result->MinimapWidth;x++)
	{
	    MinimapTexture[(x+y*Result->MinimapWidth)*4+0]=PaletteData[(MinimapData[x+y*Result->MinimapWidth])*4+0];
	    MinimapTexture[(x+y*Result->MinimapWidth)*4+1]=PaletteData[(MinimapData[x+y*Result->MinimapWidth])*4+1];
	    MinimapTexture[(x+y*Result->MinimapWidth)*4+2]=PaletteData[(MinimapData[x+y*Result->MinimapWidth])*4+2];
	    MinimapTexture[(x+y*Result->MinimapWidth)*4+3]=255;
	}
    }

    if(!Result->MinimapTexture)
	glGenTextures(1,&Result->MinimapTexture);
    glBindTexture(GL_TEXTURE_2D,Result->MinimapTexture);
    glEnable(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,Result->MinimapWidth, Result->MinimapHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, MinimapTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    PopArray(TempArena, MinimapTexture, Result->MinimapHeight* Result->MinimapWidth * 4, u8);

    //TODO(Christof): TA Apparently uses 0xDD to denote transparency, need to deal with this here

    s32 TileTextureSide = (s32)ceil(sqrt(Header->NumberOfTiles *32*32.0f));
    TileTextureSide += 32-(TileTextureSide%32);
    u8 * TileTextureData = PushArray(TempArena,TileTextureSide*TileTextureSide*4,u8 );
    memset(TileTextureData, 0, (size_t)(TileTextureSide*TileTextureSide*4));
    int i=0;

    for(int Y=0;Y<TileTextureSide;Y+=32)
    {
	for(int X=0;X<TileTextureSide;X+=32)
	{
	    for(int y=0;y<32;y++)
	    {
		for(int x=0;x<32;x++)
		{

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
	    if((u32 )i>=Header->NumberOfTiles)
		goto texture_done;
	}
    }


texture_done:

    if(!Result->MapTexture)
	glGenTextures(1,&Result->MapTexture);
    glBindTexture(GL_TEXTURE_2D,Result->MapTexture);
    glEnable(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,TileTextureSide, TileTextureSide,0, GL_RGBA, GL_UNSIGNED_BYTE, TileTextureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    PopArray(TempArena,TileTextureData,TileTextureSide*TileTextureSide*4,u8 );



    const int NUM_FLOATS_PER_HALFTILE=2*3*(3+2);//2 triangles per half tile, 3 verts per tri, 3 poscoords + 2 texcoords per vert
    GLfloat * PositionAndTexture = PushArray(TempArena,NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,GLfloat);

    float UnitsPerTile=6.0;
    //TODO(Christof): Store heightmap at least for collision stuff later?
    for(u32 X=0;X<Header->Width;X++)
    {
	for(u32 Y=0;Y<Header->Height;Y++)
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


    if(!Result->MapVertexBuffer)
    {
	glGenVertexArrays(1,&Result->MapVertexBuffer);
	glGenBuffers(1,&Result->VertexBuffer);
    }
    glBindVertexArray(Result->MapVertexBuffer);



    glBindBuffer(GL_ARRAY_BUFFER,Result->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,(s64)sizeof(GLfloat)*NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,PositionAndTexture,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*NUM_FLOATS_PER_HALFTILE/6,0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*NUM_FLOATS_PER_HALFTILE/6,(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    Result->NumTriangles = (s32)NumberOfHalfTiles*2;
    PopArray(TempArena,PositionAndTexture,NumberOfHalfTiles*NUM_FLOATS_PER_HALFTILE,GLfloat);

    return 1;
}

internal inline void UnloadTNT(TAMap * Map)
{
    if(Map && Map->MapVertexBuffer)
    {
    //glDeleteBuffers(1, &Map->MapVertexBuffer);
    //glDeleteTextures(1,&Map->MapTexture);
    }
}


internal void LoadMap(TAMap * Map, const char * OtaName, HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena, u8* PaletteData)
{
    const s32 MAX_STRING = 128;
    char MapName[MAX_STRING];

    UnloadTNT(Map);


    snprintf(MapName, MAX_STRING, "maps/%s", OtaName);
    size_t Len = strlen(MapName);
    MapName[Len-1] = 't';
    MapName[Len-2] = 'n';
    MapName[Len-3] = 't';
    HPIEntry TNT = FindEntryInAllFiles(MapName,GlobalArchiveCollection, TempArena);
    if(TNT.Name)
    {
	u8 * temp = PushArray(TempArena,TNT.File.FileSize,u8 );

	if(LoadHPIFileEntryData(TNT,temp,TempArena))
	{
	    LoadTNTFromBuffer(temp,Map,PaletteData,TempArena);
	}
	else
	    LogDebug("failed to load map buffer from hpi");
	PopArray(TempArena,temp,TNT.File.FileSize,u8 );
    }
    else
	LogDebug("failed to load map");
}
