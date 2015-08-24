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
	if(CaseInsensitiveMatch(Name, UnitTypeList->Types[i].Details.GetString("UnitName")))
	    return &UnitTypeList->Types[i];
    }
    return 0;
}
