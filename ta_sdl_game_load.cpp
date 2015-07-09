


void LoadFonts(GameState * CurrentGameState)
{
    LoadFont(CurrentGameState->Times32,"data/times.ttf",32,CurrentGameState->FontBitmap);
    LoadFont(CurrentGameState->Times24,"data/times.ttf",24,CurrentGameState->FontBitmap);
    LoadFont(CurrentGameState->Times16,"data/times.ttf",16,CurrentGameState->FontBitmap);
}




void LoadCurrentModel(GameState * CurrentGameState);
void InitialiseGame(Memory * GameMemory);

void ReloadShaders(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    if(CurrentGameState->UnitShader->ProgramID)
	UnloadShaderProgram(CurrentGameState->UnitShader);
    if(CurrentGameState->MapShader->ProgramID)
	UnloadShaderProgram(CurrentGameState->MapShader);
    LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl",CurrentGameState->UnitShader);
    
    glUseProgram(CurrentGameState->UnitShader->ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->UnitShader,"UnitTexture"),0);

    LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl",CurrentGameState->MapShader);
    
    glUseProgram(CurrentGameState->MapShader->ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->MapShader,"Texture"),0);

    CurrentGameState->ProjectionMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ProjectionMatrix");
    CurrentGameState->ModelMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ModelMatrix");
    CurrentGameState->ViewMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ViewMatrix");


    LoadShaderProgram("shaders/font.vs.glsl","shaders/font.fs.glsl",CurrentGameState->FontShader);
    glUseProgram(CurrentGameState->FontShader->ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->FontShader,"Texture"),0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

   glUniform2iv(GetUniformLocation(CurrentGameState->FontShader,"Viewport"),1,viewport+2);
    CurrentGameState->FontPositionLocation=GetUniformLocation(CurrentGameState->FontShader,"Position");
    CurrentGameState->FontColorLocation=GetUniformLocation(CurrentGameState->FontShader,"TextColor");

    
    LoadShaderProgram("shaders/UI.vs.glsl","shaders/UI.fs.glsl",CurrentGameState->UIElementShaderProgram);
    glUseProgram(CurrentGameState->UIElementShaderProgram->ProgramID);

    CurrentGameState->UIElementPositionLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Position");
    CurrentGameState->UIElementSizeLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Size");
    CurrentGameState->UIElementColorLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Color");
    CurrentGameState->UIElementBorderColorLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"BorderColor");
    CurrentGameState->UIElementBorderWidthLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"BorderWidth");
    CurrentGameState->UIElementAlphaLocation = GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Alpha");

    glUniform2iv(GetUniformLocation(CurrentGameState->UIElementShaderProgram,"Viewport"),1,viewport+2);

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
	SetupUIElementRender(CurrentGameState);

	ReloadShaders(GameMemory);
        
	LoadHPIFileCollection(CurrentGameState);
	LoadAllTextures(CurrentGameState);
	HPIEntry Map = FindEntryInAllFiles("maps/Coast To Coast.tnt",CurrentGameState);
	if(Map.Name)
	{
	    uint8_t * temp = PushArray(&CurrentGameState->TempArena,Map.File.FileSize,uint8_t);
    
	    if(LoadHPIFileEntryData(Map,temp,&CurrentGameState->TempArena))
	    {
		LoadTNTFromBuffer(temp,CurrentGameState->TestMap,CurrentGameState->PaletteData,&CurrentGameState->TempArena);
	    }
	    else
		LogDebug("failed to load map buffer from hpi");
	    PopArray(&CurrentGameState->TempArena,temp,Map.File.FileSize,uint8_t);
	}
	else
	    LogDebug("failed to load map");
    
	

	HPIEntry Entry=FindEntryInAllFiles("units",CurrentGameState);
	if(Entry.IsDirectory)
	{
	    for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
	    {
		//char temp[Entry.Directory.Entries[i].File.FileSize];
		//STACK_ARRAY(temp,Entry.Directory.Entries[i].File.FileSize,char);
		char * temp = PushArray(&CurrentGameState->TempArena, Entry.Directory.Entries[i].File.FileSize,char);
		if(LoadHPIFileEntryData(Entry.Directory.Entries[i],(uint8_t*)temp,&CurrentGameState->TempArena))
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

	Entry = FindEntryInAllFiles("fonts/ROMAN10.fnt", CurrentGameState);
	FNTFont Font;
	if(!Entry.IsDirectory)
	{
	    STACK_ARRAY(temp, Entry.File.FileSize, uint8_t);
	    if(LoadHPIFileEntryData(Entry, temp, &CurrentGameState->TempArena))
	    {
		LoadFNTFont(temp, &Font, Entry.File.FileSize, CurrentGameState);
	    }
	}
	
	CurrentGameState->StartTime= GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);
    }


    void GameTeardown(Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	int64_t EndTime=GetTimeMillis(CurrentGameState->PerformanceCounterFrequency);
	int64_t StartTime=CurrentGameState->StartTime;
	int NumberOfFrames=CurrentGameState->NumberOfFrames;
	LogDebug("%d frames in %.3fs, %.2f FPS",NumberOfFrames,(EndTime-StartTime)/1000.0,NumberOfFrames/((EndTime-StartTime)/1000.0));
	UnloadShaderProgram(CurrentGameState->UnitShader);
	UnloadHPIFileCollection(CurrentGameState);
	Unload3DO(CurrentGameState->temp_model);
    }




    void CheckResources(Memory * GameMemory)
    {
	//return;
	//TODO(Christof): Fix shaders unecessarily reloading here (cause of the blue flickering)
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	uint64_t UnitVertexShaderTime = GetFileModifiedTime("shaders/unit3do.vs.glsl");
	uint64_t UnitPixelShaderTime = GetFileModifiedTime("shaders/unit3do.fs.glsl");
	uint64_t MapVertexShaderTime = GetFileModifiedTime("shaders/map.vs.glsl");
	uint64_t MapPixelShaderTime = GetFileModifiedTime("shaders/map.fs.glsl");

	uint64_t UIVertexShaderTime = GetFileModifiedTime("shaders/UI.vs.glsl");
	uint64_t UIPixelShaderTime = GetFileModifiedTime("shaders/UI.fs.glsl");

	uint64_t TextVertexShaderTime = GetFileModifiedTime("shaders/font.vs.glsl");
	uint64_t TextPixelShaderTime = GetFileModifiedTime("shaders/font.fs.glsl");
	if(UnitVertexShaderTime > CurrentGameState->UnitShader->VertexFileModifiedTime || UnitPixelShaderTime > CurrentGameState->UnitShader->PixelFileModifiedTime
	   || MapPixelShaderTime > CurrentGameState->MapShader->VertexFileModifiedTime || MapVertexShaderTime > CurrentGameState->MapShader->PixelFileModifiedTime
	   || UIVertexShaderTime > CurrentGameState->UIElementShaderProgram->VertexFileModifiedTime || UIPixelShaderTime > CurrentGameState->UIElementShaderProgram->PixelFileModifiedTime
	   || TextVertexShaderTime > CurrentGameState->FontShader->VertexFileModifiedTime || TextPixelShaderTime > CurrentGameState->FontShader->PixelFileModifiedTime
	    )
	    ReloadShaders(GameMemory);
    }
    
}
