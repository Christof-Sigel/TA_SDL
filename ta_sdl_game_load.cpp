


void LoadFonts(GameState * CurrentGameState)
{
    LoadFont(&CurrentGameState->Times32,"data/times.ttf",32,CurrentGameState->FontBitmap);
    LoadFont(&CurrentGameState->Times24,"data/times.ttf",24,CurrentGameState->FontBitmap);
    LoadFont(&CurrentGameState->Times16,"data/times.ttf",16,CurrentGameState->FontBitmap);
}




void LoadCurrentModel(GameState * CurrentGameState);
void InitialiseGame(Memory * GameMemory);

void ReloadShaders(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    UnloadAllShaders(&CurrentGameState->ShaderGroup);
    CurrentGameState->UnitShader = LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl",&CurrentGameState->ShaderGroup);
    if(CurrentGameState->UnitShader->ProgramID)
    {
	glUseProgram(CurrentGameState->UnitShader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->UnitShader,"UnitTexture"),0);
        CurrentGameState->ProjectionMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ProjectionMatrix");
	CurrentGameState->ModelMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ModelMatrix");
	CurrentGameState->ViewMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ViewMatrix");

    }

    CurrentGameState->MapShader=LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl",&CurrentGameState->ShaderGroup);

    if(CurrentGameState->MapShader->ProgramID)
    {
	glUseProgram(CurrentGameState->MapShader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->MapShader,"Texture"),0);
    }


    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    CurrentGameState->FontShader= LoadShaderProgram("shaders/font.vs.glsl","shaders/font.fs.glsl",&CurrentGameState->ShaderGroup);
    if(CurrentGameState->FontShader->ProgramID)
    {
	glUseProgram(CurrentGameState->FontShader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->FontShader,"Texture"),0);

	glUniform2iv(GetUniformLocation(CurrentGameState->FontShader,"Viewport"),1,viewport+2);
	CurrentGameState->FontPositionLocation=GetUniformLocation(CurrentGameState->FontShader,"Position");
	CurrentGameState->FontColorLocation=GetUniformLocation(CurrentGameState->FontShader,"TextColor");
    }

    
    CurrentGameState->UIElementShaderProgram = LoadShaderProgram("shaders/UI.vs.glsl","shaders/UI.fs.glsl",&CurrentGameState->ShaderGroup);
    if(CurrentGameState->UIElementShaderProgram->ProgramID)
    {
	glUseProgram(CurrentGameState->UIElementShaderProgram->ProgramID);

	CurrentGameState->UIElementPositionLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Position");
	CurrentGameState->UIElementSizeLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Size");
	CurrentGameState->UIElementColorLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Color");
	CurrentGameState->UIElementBorderColorLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"BorderColor");
	CurrentGameState->UIElementBorderWidthLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"BorderWidth");
	CurrentGameState->UIElementAlphaLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Alpha");

	glUniform2iv(GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Viewport"),1,viewport+2);
    }


    FontShaderDetails * FDetails = &CurrentGameState->FontShaderDetails;
    FDetails->Program = LoadShaderProgram("shaders/TAFont.vs.glsl","shaders/TAFont.fs.glsl",&CurrentGameState->ShaderGroup);
    if(FDetails->Program->ProgramID)
    {
	FDetails->ColorLocation=GetUniformLocation(FDetails->Program,"Color");
	FDetails->AlphaLocation=GetUniformLocation(FDetails->Program,"Alpha");
	FDetails->PositionLocation=GetUniformLocation(FDetails->Program,"Position");
	FDetails->SizeLocation=GetUniformLocation(FDetails->Program,"Size");
	FDetails->TextureOffsetLocation=GetUniformLocation(FDetails->Program,"TextureOffset");
    
	glUniform1i(GetUniformLocation(FDetails->Program,"Texture"), 0);
	glUniform2iv(GetUniformLocation(FDetails->Program,"Viewport"),1,viewport+2);
    }

    Texture2DShaderDetails * TDetails = &CurrentGameState->DrawTextureShaderDetails;
    TDetails->Program = LoadShaderProgram("shaders/2DRender.vs.glsl","shaders/2DRender.fs.glsl",&CurrentGameState->ShaderGroup);
    if(TDetails->Program->ProgramID)
    {
	TDetails->ColorLocation=GetUniformLocation(TDetails->Program,"Color");
	TDetails->AlphaLocation=GetUniformLocation(TDetails->Program,"Alpha");
	TDetails->PositionLocation=GetUniformLocation(TDetails->Program,"Position");
	TDetails->SizeLocation=GetUniformLocation(TDetails->Program,"Size");
	TDetails->TextureOffsetLocation=GetUniformLocation(TDetails->Program,"TextureOffset");
	TDetails->TextureSizeLocation=GetUniformLocation(TDetails->Program,"TextureSize");
    
	glUniform1i(GetUniformLocation(TDetails->Program,"Texture"), 0);
	glUniform2iv(GetUniformLocation(TDetails->Program,"Viewport"),1,viewport+2);
    }
    
}

void SetupGameState( GameState * CurrentGameState);
extern "C"{
    void GameSetup(Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	if(!CurrentGameState->IsInitialised)
	{
	    InitialiseGame(GameMemory);
	}
#ifdef __WINDOWS__
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	PerformaceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
	
	LoadFonts(CurrentGameState);
	SetupUIElementDrawing(CurrentGameState);

	ReloadShaders(GameMemory);
        
	LoadHPIFileCollection(&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->GameArena, &CurrentGameState->TempArena);	
	LoadPalette(CurrentGameState);
	
	SetupTextureContainer(&CurrentGameState->UnitTextures, UNIT_TEXTURE_WIDTH, UNIT_TEXTURE_HEIGHT, UNIT_MAX_TEXTURES, &CurrentGameState->GameArena);
	LoadCommonUITextures(CurrentGameState);
	LoadGafFonts(CurrentGameState);
	

	LoadAllUnitTextures(&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->UnitTextures, CurrentGameState->PaletteData);

	SetupFontRendering(&CurrentGameState->Draw2DVertexBuffer);
	CurrentGameState->DrawTextureShaderDetails.VertexBuffer = CurrentGameState->Draw2DVertexBuffer;
	
	HPIEntry Map = FindEntryInAllFiles("maps/Coast To Coast.tnt",&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
	if(Map.Name)
	{
	    u8 * temp = PushArray(&CurrentGameState->TempArena,Map.File.FileSize,u8 );
    
	    if(LoadHPIFileEntryData(Map,temp,&CurrentGameState->TempArena))
	    {
		LoadTNTFromBuffer(temp,&CurrentGameState->TestMap,CurrentGameState->PaletteData,&CurrentGameState->TempArena);
	    }
	    else
		LogDebug("failed to load map buffer from hpi");
	    PopArray(&CurrentGameState->TempArena,temp,Map.File.FileSize,u8 );
	}
	else
	    LogDebug("failed to load map");
    
	

	HPIEntry Entry=FindEntryInAllFiles("units",&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
	if(Entry.IsDirectory)
	{
	    for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
	    {
		//char temp[Entry.Directory.Entries[i].File.FileSize];
		//STACK_ARRAY(temp,Entry.Directory.Entries[i].File.FileSize,char);
		char * temp = PushArray(&CurrentGameState->TempArena, Entry.Directory.Entries[i].File.FileSize,char);
		if(LoadHPIFileEntryData(Entry.Directory.Entries[i],(u8 *)temp,&CurrentGameState->TempArena))
		{
		    if(strstr(Entry.Directory.Entries[i].Name,".FBI"))
		    {
			if(CurrentGameState->Units.Size>=MAX_UNITS_LOADED)
			{
			    LogError("TOO MANY UNITS");
			}
			else
			{
			    LoadFBIFileFromBuffer(&CurrentGameState->Units.Details[CurrentGameState->Units.Size++],temp,&CurrentGameState->GameArena);
			}
		    }
		}
		PopArray(&CurrentGameState->TempArena, temp,Entry.Directory.Entries[i].File.FileSize,char);
	    }
	}
	UnloadCompositeEntry(&Entry,&CurrentGameState->TempArena);

	LoadCurrentModel(CurrentGameState);

	Entry = FindEntryInAllFiles("fonts/ARMFONT.fnt", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
	if(!Entry.IsDirectory)
	{
	    STACK_ARRAY(temp, Entry.File.FileSize, u8 );
	    if(LoadHPIFileEntryData(Entry, temp, &CurrentGameState->TempArena))
	    {
		LoadFNTFont(temp, &CurrentGameState->Fonts[0], Entry.File.FileSize);
	    }
	}

	Entry = FindEntryInAllFiles("guis/mainmenu.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
	if(!Entry.IsDirectory)
	{
	    STACK_ARRAY(temp, Entry.File.FileSize, u8 );
	    if(LoadHPIFileEntryData(Entry, temp, &CurrentGameState->TempArena))
	    {
		CurrentGameState->MainGUI = LoadGUIFromBuffer((char*)temp, (char*)temp+Entry.File.FileSize, &CurrentGameState->GameArena, &CurrentGameState->TempArena,Entry.Name, &CurrentGameState->GlobalArchiveCollection, CurrentGameState->PaletteData);
	    }
	}

	CurrentGameState->StartTime= GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);
    }


    void GameTeardown(Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	s64  EndTime=GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);
	s64  StartTime=CurrentGameState->StartTime;
	int NumberOfFrames=CurrentGameState->NumberOfFrames;
	LogDebug("%d frames in %.3fs, %.2f FPS",NumberOfFrames,(EndTime-StartTime)/1000.0,NumberOfFrames/((EndTime-StartTime)/1000.0));
	UnloadShaderProgram(CurrentGameState->UnitShader);
	UnloadHPIFileCollection(&CurrentGameState->GlobalArchiveCollection);
	Unload3DO(&CurrentGameState->temp_model);
    }




    void CheckResources(Memory * GameMemory)
    {
	//return;
	//TODO(Christof): Fix shaders unecessarily reloading here (cause of the blue flickering)
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	u8 Reload =0;
	for(int i=0;i<CurrentGameState->ShaderGroup.NumberOfShaders;i++)
	{
	    if(GetFileModifiedTime(CurrentGameState->ShaderGroup.Shaders[i].VertexFileName) > CurrentGameState->ShaderGroup.Shaders[i].VertexFileModifiedTime)
	    {
		Reload=1;
		break;
	    }
	    if(GetFileModifiedTime(CurrentGameState->ShaderGroup.Shaders[i].PixelFileName) > CurrentGameState->ShaderGroup.Shaders[i].PixelFileModifiedTime)
	    {
		Reload=1;
		break;
	    }
	}
	if(Reload)
	    ReloadShaders(GameMemory);
    }
    
}
