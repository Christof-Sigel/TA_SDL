internal void LoadAllUnitTypes(UnitTypeList * UnitTypeList, MemoryArena * GameArena, MemoryArena * TempArena, HPIFileCollection * GlobalArchiveCollection, TextureContainer * UnitTextures)
{
    memset(UnitTypeList,0,sizeof(struct UnitTypeList));
    HPIEntry UnitsEntry=FindEntryInAllFiles("units",GlobalArchiveCollection, TempArena);
    if(UnitsEntry.IsDirectory)
    {
	for(int i=0;i<UnitsEntry.Directory.NumberOfEntries;i++)
	{
	    char * UnitBuffer = PushArray(TempArena, UnitsEntry.Directory.Entries[i].File.FileSize,char);
	    if(LoadHPIFileEntryData(UnitsEntry.Directory.Entries[i],(u8 *)UnitBuffer,TempArena))
	    {
		if(strstr(UnitsEntry.Directory.Entries[i].Name,".FBI"))
		{
		    UnitType * UnitType = &UnitTypeList->Types[UnitTypeList->NumberOfUnitTypes++];
		    if(UnitTypeList->NumberOfUnitTypes>=MAX_UNIT_TYPES)
		    {
			LogError("TOO MANY UNITS");
		    }
		    else
		    {
			LoadFBIFileFromBuffer(&UnitType->Details,UnitBuffer,GameArena);
		    }

		    char * UnitName=UnitType->Details.GetString("UnitName");
		    const int MAX_MODEL_NAME=64;
		    char Name[MAX_MODEL_NAME];
		    snprintf(Name,MAX_MODEL_NAME,"objects3d/%s.3do",UnitName);

		    HPIEntry ModelEntry=FindEntryInAllFiles(Name,GlobalArchiveCollection, TempArena);
		    u8 * ModelBuffer = PushArray(TempArena,ModelEntry.File.FileSize,u8 );
		    if(LoadHPIFileEntryData(ModelEntry,ModelBuffer,TempArena))
		    {
			if(UnitType->Model.Vertices)
			{
			    Unload3DO(&UnitType->Model);
			}
			Load3DOFromBuffer(ModelBuffer,&UnitType->Model,UnitTextures,GameArena);

			snprintf(Name,MAX_MODEL_NAME,"scripts/%s.cob",UnitName);
			HPIEntry ScriptEntry=FindEntryInAllFiles(Name,GlobalArchiveCollection, TempArena);
			u8 * ScriptBuffer = PushArray(TempArena, ScriptEntry.File.FileSize, u8 );
			if(LoadHPIFileEntryData(ScriptEntry,ScriptBuffer,TempArena))
			{
			    LoadUnitScriptFromBuffer(&UnitType->Script, ScriptBuffer,GameArena);
			}
			PopArray(TempArena, ScriptBuffer,ScriptEntry.File.FileSize, u8 );

		    }
		    PopArray(TempArena, ModelBuffer,ModelEntry.File.FileSize,u8 );


		}
	    }
	    PopArray(TempArena, UnitBuffer,UnitsEntry.Directory.Entries[i].File.FileSize,char);
	}
    }
    UnloadCompositeEntry(&UnitsEntry,TempArena);
}

internal UnitType * GetUnitType(const char * Name, UnitTypeList * UnitTypeList)
{
    for(s32 i=0;i<UnitTypeList->NumberOfUnitTypes;i++)
    {
	char * UnitType = UnitTypeList->Types[i].Details.GetString("UnitName");
	if(CaseInsensitiveMatch(Name, UnitType))
	    return &UnitTypeList->Types[i];
    }
    return 0;
}



internal Unit * CreateNewUnit(const char * UnitTypeName, UnitTypeList * UnitTypeList, MemoryArena * UnitArena, Unit * UnitList, s32 * NumberOfUnits, TAMap * Map, r32 X, r32 Y)
{
     s32 MapX = X/16;
    s32 MapY = Y/16;
    X = X /16*GL_UNIT_PER_MAP_TILE;
    Y = Y /16*GL_UNIT_PER_MAP_TILE;

    Assert(*NumberOfUnits < MAX_TOTAL_UNITS);
    UnitType * Type = GetUnitType(UnitTypeName, UnitTypeList);
    Unit * Result = &UnitList[(*NumberOfUnits)++];
    *Result = {};
    Result->X = X;
    Result->Y = Y;

    if(MapX >= Map->Width)
	MapX = Map->Width -1;
    if(MapX <0)
	MapX = 0;
    if(MapY < 0)
	MapY =0;
    if(MapY >= Map->Height)
	MapY = Map->Height -1;
    Result->Z = Map->HeightMap[MapX + MapY*Map->Width];
    Result->Type = Type;
    InitTransformationDetails(&Type->Model, &Result->TransformationDetails, UnitArena);
    StartNewEntryPoint(&Result->ScriptPool, &Type->Script, "Create", 0,0, &Result->TransformationDetails);
    return Result;
}


internal void UpdateAndRenderUnit(Unit * Unit, b32 Animate, UnitShaderDetails * UnitShaderDetails, u8* PaletteData, TextureContainer * UnitTextures, MemoryArena * TempArena, GLuint DebugAxisBuffer)
{
    for(int i=0;i<Unit->ScriptPool.NumberOfScripts;i++)
    {
	RunScript(&Unit->Type->Script, &Unit->ScriptPool.Scripts[i], &Unit->Type->Model, &Unit->ScriptPool);
    }
    CleanUpScriptPool(&Unit->ScriptPool);

    UpdateTransformationDetails(&Unit->Type->Model,&Unit->TransformationDetails,1.0f/60.0f, Animate);

    Matrix ModelMatrix;


//    ModelMatrix.SetTranslation(Unit->X,44.5f,Unit->Y);

    r32 XRot = -0.15*PI;
    //if(Unit->Type->Details.GetInt("Upright") != 1)
	XRot =0;
    ModelMatrix.Rotate(0,1,0, Unit->Rotation[1]);//Unit->Rotation[1]);
    ModelMatrix.Rotate(1,0,0,  Unit->Rotation[0]+XRot);
    ModelMatrix.Rotate(0,0,1,  Unit->Rotation[2]);

    ModelMatrix.Move(Unit->X,Unit->Z,Unit->Y );
    RenderObject3d(&Unit->Type->Model,&Unit->TransformationDetails,UnitShaderDetails->ModelMatrixLocation,PaletteData,DebugAxisBuffer, Unit->Side, UnitTextures,TempArena,Matrix(),ModelMatrix);

}
