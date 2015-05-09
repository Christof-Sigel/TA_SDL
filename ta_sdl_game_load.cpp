

FontDetails Times32;
FontDetails Times24;
FontDetails Times16;

void LoadFonts()
{
    Times32=LoadFont("data/times.ttf",32);
    Times24=LoadFont("data/times.ttf",24);
    Times16=LoadFont("data/times.ttf",16);
}


TAMap TestMap={0};
std::vector<UnitDetails> Units;
void LoadCurrentModel(GameState * CurrentGameState);

extern int ScreenWidth;
extern int ScreenHeight;

void ReloadShaders(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    if(CurrentGameState->UnitShader.ProgramID)
	UnloadShaderProgram(CurrentGameState->UnitShader);
    if(CurrentGameState->MapShader.ProgramID)
	UnloadShaderProgram(CurrentGameState->MapShader);
    CurrentGameState->UnitShader=LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl");
    
    glUseProgram(CurrentGameState->UnitShader.ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->UnitShader,"UnitTexture"),0);

    CurrentGameState->MapShader=LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl");
    
    glUseProgram(CurrentGameState->MapShader.ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->MapShader,"Texture"),0);

    CurrentGameState->ProjectionMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ProjectionMatrix");
    CurrentGameState->ModelMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ModelMatrix");
    CurrentGameState->ViewMatrixLocation = GetUniformLocation(CurrentGameState->UnitShader,"ViewMatrix");


    CurrentGameState->FontShader=LoadShaderProgram("shaders/font.vs.glsl","shaders/font.fs.glsl");
    glUseProgram(CurrentGameState->FontShader.ProgramID);
    glUniform1i(GetUniformLocation(CurrentGameState->FontShader,"Texture"),0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glUniform2iv(GetUniformLocation(CurrentGameState->FontShader,"Viewport"),1,viewport+2);
    CurrentGameState->FontPositionLocation=GetUniformLocation(CurrentGameState->FontShader,"Position");
    CurrentGameState->FontColorLocation=GetUniformLocation(CurrentGameState->FontShader,"TextColor");

    
    UIElementShaderProgram=LoadShaderProgram("shaders/UI.vs.glsl","shaders/UI.fs.glsl");
    glUseProgram(UIElementShaderProgram.ProgramID);

    UIElementPositionLocation = GetUniformLocation(UIElementShaderProgram,"Position");
    UIElementSizeLocation = GetUniformLocation(UIElementShaderProgram,"Size");
    UIElementColorLocation = GetUniformLocation(UIElementShaderProgram,"Color");
    UIElementBorderColorLocation = GetUniformLocation(UIElementShaderProgram,"BorderColor");
    UIElementBorderWidthLocation = GetUniformLocation(UIElementShaderProgram,"BorderWidth");
    UIElementAlphaLocation = GetUniformLocation(UIElementShaderProgram,"Alpha");

    glUniform2iv(GetUniformLocation(UIElementShaderProgram,"Viewport"),1,viewport+2);

}

void GameSetup(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
#ifdef __WINDOWS__
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerformaceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    LoadFonts();
    SetupUIElementRender();
    
    // ViewMatrix.Rotate(0,1,0, PI);
    //ViewMatrix.Rotate(0,1,0, -PI/4);
    //ViewMatrix.Move(1,0,0);
    
    ReloadShaders(GameMemory);

        
    LoadHPIFileCollection();
    LoadAllTextures();
    HPIEntry Map = FindEntryInAllFiles("maps/Greenhaven.tnt");
    if(Map.Name)
    {
	char * temp = (char*)malloc(Map.File.FileSize);
    
	if(LoadHPIFileEntryData(Map,temp))
	{
	    TestMap=LoadTNTFromBuffer(temp);
	}
	else
	    LogDebug("failed to load map buffer from hpi");
	free(temp);
    }
    else
	LogDebug("failed to load map");
    
	

    HPIEntry Entry=FindEntryInAllFiles("units");
    if(Entry.IsDirectory)
    {
	for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
	{
	    char temp[Entry.Directory.Entries[i].File.FileSize];
	    if(LoadHPIFileEntryData(Entry.Directory.Entries[i],temp))
	    {
		UnitDetails deets;
		if(strstr(Entry.Directory.Entries[i].Name,".FBI"))
		{
		    LoadFBIFileFromBuffer(&deets,temp);
		    Units.push_back(deets);
		}
	    }
	}
    }
    UnloadCompositeEntry(&Entry);

    LoadCurrentModel(CurrentGameState);

    CurrentGameState->StartTime= GetTimeMillis();
}


void GameTeardown(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    int64_t EndTime=GetTimeMillis();
    int64_t StartTime=CurrentGameState->StartTime;
    int NumberOfFrames=CurrentGameState->NumberOfFrames;
    LogDebug("%d frames in %.3fs, %.2f FPS",NumberOfFrames,(EndTime-StartTime)/1000.0,NumberOfFrames/((EndTime-StartTime)/1000.0));
    UnloadShaderProgram(CurrentGameState->UnitShader);
    UnloadHPIFileCollection();
    Unload3DO(&CurrentGameState->temp_model);
}




void CheckResources(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    uint64_t UnitVertexShaderTime = GetFileModifiedTime("shaders/unit3do.vs.glsl");
    uint64_t UnitPixelShaderTime = GetFileModifiedTime("shaders/unit3do.fs.glsl");
    uint64_t MapVertexShaderTime = GetFileModifiedTime("shaders/map.vs.glsl");
    uint64_t MapPixelShaderTime = GetFileModifiedTime("shaders/map.fs.glsl");

    uint64_t UIVertexShaderTime = GetFileModifiedTime("shaders/UI.vs.glsl");
    uint64_t UIPixelShaderTime = GetFileModifiedTime("shaders/UI.fs.glsl");

    uint64_t TextVertexShaderTime = GetFileModifiedTime("shaders/font.vs.glsl");
    uint64_t TextPixelShaderTime = GetFileModifiedTime("shaders/font.fs.glsl");
    if(UnitVertexShaderTime > CurrentGameState->UnitShader.VertexFileModifiedTime || UnitPixelShaderTime > CurrentGameState->UnitShader.PixelFileModifiedTime
       || MapPixelShaderTime > CurrentGameState->MapShader.VertexFileModifiedTime || MapVertexShaderTime > CurrentGameState->MapShader.PixelFileModifiedTime
       || UIVertexShaderTime > UIElementShaderProgram.VertexFileModifiedTime || UIPixelShaderTime > UIElementShaderProgram.PixelFileModifiedTime
       || TextVertexShaderTime > CurrentGameState->FontShader.VertexFileModifiedTime || TextPixelShaderTime > CurrentGameState->FontShader.PixelFileModifiedTime
	)
    ReloadShaders(GameMemory);
}
    
