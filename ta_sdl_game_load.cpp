internal void InitialiseGame(Memory * GameMemory);

internal void ReloadShaders(Memory * GameMemory)
{
    GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
    UnloadAllShaders(&CurrentGameState->ShaderGroup);
    CurrentGameState->UnitShaderDetails.Shader = LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl",&CurrentGameState->ShaderGroup);
    if(CurrentGameState->UnitShaderDetails.Shader->ProgramID)
    {
	glUseProgram(CurrentGameState->UnitShaderDetails.Shader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->UnitShaderDetails.Shader,"UnitTexture"),0);
        CurrentGameState->UnitShaderDetails.ProjectionMatrixLocation = GetUniformLocation(CurrentGameState->UnitShaderDetails.Shader,"ProjectionMatrix");
	CurrentGameState->UnitShaderDetails.ModelMatrixLocation = GetUniformLocation(CurrentGameState->UnitShaderDetails.Shader,"ModelMatrix");
	CurrentGameState->UnitShaderDetails.ViewMatrixLocation = GetUniformLocation(CurrentGameState->UnitShaderDetails.Shader,"ViewMatrix");
    }

    CurrentGameState->UnitBuildShaderDetails.Shader = LoadShaderProgram("shaders/unit3dobuild.vs.glsl","shaders/unit3dobuild.fs.glsl",&CurrentGameState->ShaderGroup);
    if(CurrentGameState->UnitBuildShaderDetails.Shader->ProgramID)
    {
	glUseProgram(CurrentGameState->UnitBuildShaderDetails.Shader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->UnitBuildShaderDetails.Shader,"UnitTexture"),0);
        CurrentGameState->UnitBuildShaderDetails.ProjectionMatrixLocation = GetUniformLocation(CurrentGameState->UnitBuildShaderDetails.Shader,"ProjectionMatrix");
CurrentGameState->UnitBuildShaderDetails.ModelMatrixLocation = GetUniformLocation(CurrentGameState->UnitBuildShaderDetails.Shader,"ModelMatrix");
CurrentGameState->UnitBuildShaderDetails.ViewMatrixLocation = GetUniformLocation(CurrentGameState->UnitBuildShaderDetails.Shader,"ViewMatrix");

    }


    CurrentGameState->MapShader=LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl",&CurrentGameState->ShaderGroup);

    if(CurrentGameState->MapShader->ProgramID)
    {
	glUseProgram(CurrentGameState->MapShader->ProgramID);
	glUniform1i(GetUniformLocation(CurrentGameState->MapShader,"Texture"),0);
    }

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

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


    DebugRectShaderDetails * RDetails = &CurrentGameState->DebugRectDetails;
    RDetails->Program = LoadShaderProgram("shaders/DebugRect.vs.glsl","shaders/DebugRect.fs.glsl",&CurrentGameState->ShaderGroup);
    if(RDetails->Program->ProgramID)
    {
	RDetails->ColorLocation=GetUniformLocation(RDetails->Program,"Color");
	RDetails->PositionLocation=GetUniformLocation(RDetails->Program,"Position");
	RDetails->SizeLocation=GetUniformLocation(RDetails->Program,"Size");
	RDetails->BorderWidthLocation=GetUniformLocation(RDetails->Program,"BorderWidth");
	RDetails->BorderColorLocation=GetUniformLocation(RDetails->Program,"BorderColor");

	glUniform2iv(GetUniformLocation(RDetails->Program,"Viewport"),1,viewport+2);
    }
}

internal void SetupDebugRectBuffer(GLuint * DebugRectBuffer);

internal void LoadCampaigns(HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena, MemoryArena * CampaignArena, CampaignList * Campaigns)
{
    HPIEntry CampaignDirectory = FindEntryInAllFiles("camps", GlobalArchiveCollection, TempArena);

    if(!CampaignDirectory.IsDirectory)
    {
	LogError("File encountered while loading campaigns directory");
    }
    else
    {
	for(int i=0;i<CampaignDirectory.Directory.NumberOfEntries;i++)
	{
	    if(!CampaignDirectory.Directory.Entries[i].IsDirectory)
	    {
		HPIEntry * Entry = &CampaignDirectory.Directory.Entries[i];
		char * CampaignFileBuffer = PushArray(TempArena, Entry->File.FileSize, char);
		if(LoadHPIFileEntryData(*Entry, (u8*)CampaignFileBuffer, TempArena))
		{
		    MemoryArena * Arena = PushSubArena(TempArena, 64*1024);
		    TDFElement * First = LoadTDFElementsFromBuffer(&CampaignFileBuffer, CampaignFileBuffer + Entry->File.FileSize,Arena);
		    char * Side = GetStringValue(First,"campaignside");
		    Campaign * Campaign;
		    if(CaseInsensitiveMatch(Side,"ARM"))
		    {
			Campaign = &Campaigns->ARMCampaigns[Campaigns->NumberOfARMCampaigns++];
		    }
		    else
		    {
			//NOTE(Christof): Assuming not ARM means CORE
			Campaign = &Campaigns->CORECampaigns[Campaigns->NumberOfCORECampaigns++];
		    }
		    size_t NameLen = strlen(Entry->Name);
		    Campaign->CampaignName = PushArray(CampaignArena, NameLen, char);
		    memcpy(Campaign->CampaignName, Entry->Name, NameLen);
		    Campaign->CampaignName[NameLen -4] =0;

		    Campaign->NumberOfMissions = CountElements(First) - 1;
		    Campaign->Missions = PushArray(CampaignArena, Campaign->NumberOfMissions, Mission);
		    for(s32 j = 0;j<Campaign->NumberOfMissions;j++)
		    {
			TDFElement * Mission = GetNthElement(First, j+1);
			char * FileName = GetStringValue(Mission, "missionfile");
			char * MissionName = GetStringValue(Mission, "missionname");
			NameLen = strlen(FileName)+1;
			Campaign->Missions[j].MissionFile = PushArray(CampaignArena, NameLen, char);
			memcpy(Campaign->Missions[j].MissionFile, FileName, NameLen);

			NameLen = strlen(MissionName)+1;
			Campaign->Missions[j].MissionName = PushArray(CampaignArena, NameLen, char);
			memcpy(Campaign->Missions[j].MissionName, MissionName, NameLen);
		    }


		    PopSubArena(TempArena, Arena);
		}
//		if(CampaignFileBuffer)
//		PopArray(TempArena, CampaignFileBuffer, Entry->File.FileSize, char);
	    }
	}
    }


    //UnloadCompositeEntry(&CampaignDirectory, TempArena);
}



internal void SetupGameState( GameState * CurrentGameState);
extern "C"{

    void GameSetup(Memory * GameMemory);

    void GameSetup(Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	if(!CurrentGameState->IsInitialised)
	{
	    InitialiseGame(GameMemory);
	}
	ReloadShaders(GameMemory);

	LoadHPIFileCollection(&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->GameArena, &CurrentGameState->TempArena);
	LoadPalette(CurrentGameState);

	SetupTextureContainer(&CurrentGameState->UnitTextures, UNIT_TEXTURE_WIDTH, UNIT_TEXTURE_HEIGHT, UNIT_MAX_TEXTURES, &CurrentGameState->GameArena);
	LoadCommonUITextures(CurrentGameState);
	LoadGafFonts(CurrentGameState);


	LoadAllUnitTextures(&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->UnitTextures, CurrentGameState->PaletteData);

	SetupFontRendering(&CurrentGameState->Draw2DVertexBuffer);
	CurrentGameState->DrawTextureShaderDetails.VertexBuffer = CurrentGameState->Draw2DVertexBuffer;


	CurrentGameState->MainMenu = LoadGUI("MainMenu.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);
	CurrentGameState->SinglePlayerMenu = LoadGUI("Single.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);

	CurrentGameState->CampaignMenu = LoadGUI("NewGame.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);
	CurrentGameState->LoadGameMenu = LoadGUI("LoadGame.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);
	CurrentGameState->SkirmishMenu = LoadGUI("Skirmish.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);
	CurrentGameState->OptionsMenu = LoadGUI("startopt.gui", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena , CurrentGameState->PaletteData, &CurrentGameState->LoadedFonts);
	SetupDebugRectBuffer(&CurrentGameState->DebugRectDetails.VertexBuffer);
	LoadCampaigns(&CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena, &CurrentGameState->GameArena, &CurrentGameState->CampaignList);
    }

    void GameTeardown(Memory * GameMemory);
    void GameTeardown(Memory * GameMemory)
    {
	GameState * CurrentGameState = (GameState*)GameMemory->PermanentStore;
	UnloadHPIFileCollection(&CurrentGameState->GlobalArchiveCollection);
    }
    void CheckResources(Memory * GameMemory);
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
