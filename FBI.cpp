
enum UnitSide
{
    SIDE_ARM,
    SIDE_CORE,
    SIDE_UNKNOWN
};

struct UnitKeyValue
{
    char * Name, *Value;
};

enum
{
    UNIT_CATEGORY_ARM=0,
    UNIT_CATEGORY_KBOT,
    UNIT_CATEGORY_LEVEL2,
    UNIT_CATEGORY_CONSTRUCTOR,
    UNIT_CATEGORY_NO_WEAPON,//4
    UNIT_CATEGORY_NOT_AIR,
    UNIT_CATEGORY_NOT_SUB,
    UNIT_CATEGORY_CTRL_B,
    UNIT_CATEGORY_SHIP,//8
    UNIT_CATEGORY_LEVEL1,
    UNIT_CATEGORY_LEVEL3,
    UNIT_CATEGORY_TANK,
    UNIT_CATEGORY_WEAPON,//12
    UNIT_CATEGORY_CTRL_W,
    UNIT_CATEGORY_SPECIAL,
    UNIT_CATEGORY_SONAR,
    UNIT_CATEGORY_CORE,//16
    UNIT_CATEGORY_LEVEL10,
    UNIT_CATEGORY_CTRL_C,
    UNIT_CATEGORY_ENERGY,
    UNIT_CATEGORY_METAL, //20
    UNIT_CATEGORY_PARAL,
    UNIT_CATEGORY_COMMANDER,
    UNIT_CATEGORY_CARRY,
    UNIT_CATEGORY_CTRL_F,//24
    UNIT_CATEGORY_PLANT,
    UNIT_CATEGORY_RAD,
    UNIT_CATEGORY_TORP,
    UNIT_CATEGORY_CTRL_V,//28
    UNIT_CATEGORY_VTOL,
    UNIT_CATEGORY_STRATEGIC,
    UNIT_CATEGORY_DEFENSIVE,//32
    UNIT_CATEGORY_JAM,
    UNIT_CATEGORY_REPAIR_PAD,
    UNIT_CATEGORY_ANTI_SUB,
    UNIT_CATEGORY_STEALTH,//36
    UNIT_CATEGORY_BOMB,
    UNIT_CATEGORY_PHIB,
    UNIT_CATEGORY_UNDERWATER,
    UNIT_CATEGORY_KAMIKAZE//40
};

#define MAX_UNIT_DETAILS 128

struct UnitDetails
{
    UnitKeyValue Details[MAX_UNIT_DETAILS];
    int DetailsSize;
    int GetInt(const char * Name)
    {
	char * Value=GetString(Name);
	if(Value)
	    return atoi(Value);
	return -1;
    }
    
    float GetFloat(const char * Name)
    {
	char * Value= GetString(Name);
	if(Value)
	    return (float)atof(Value);
	return -1;
    }
    
    char * GetString(const char * Name)
    {
	for(int i=0;i<DetailsSize;i++)
	    if(CaseInsensitiveMatch(Details[i].Name,Name))
		return Details[i].Value;
	return 0;
    }
    
    int64_t GetUnitCategories()
    {
	char * Value=GetString("Category");
	if(!Value)
	    return 0;
	
	int64_t Categories=0;
	int done=0;
	while(!done)
	{
	    char * Cat=Value;
	    while(*Value && *Value !=' '){Value++;}
	    if(!*Value)
		done=1;
	    if(Cat==Value)
		break;
	    *Value=0;
	    if(CaseInsensitiveMatch(Cat,"ARM")){Categories |= 1<<UNIT_CATEGORY_ARM;}
	    else if(CaseInsensitiveMatch(Cat,"KBOT")){Categories |= 1<<UNIT_CATEGORY_KBOT;}
	    else if(CaseInsensitiveMatch(Cat,"LEVEL2")){Categories |= 1<<UNIT_CATEGORY_LEVEL2;}
	    else if(CaseInsensitiveMatch(Cat,"CONSTR")){Categories |= 1<<UNIT_CATEGORY_CONSTRUCTOR;}
	    else if(CaseInsensitiveMatch(Cat,"NOWEAPON")){Categories |= 1<<UNIT_CATEGORY_NO_WEAPON;}
	    else if(CaseInsensitiveMatch(Cat,"NOTAIR")){Categories |= 1<<UNIT_CATEGORY_NOT_AIR;}
	    else if(CaseInsensitiveMatch(Cat,"NOTSUB")){Categories |= 1<<UNIT_CATEGORY_NOT_SUB;}
	    else if(CaseInsensitiveMatch(Cat,"CTRL_B")){Categories |= 1<<UNIT_CATEGORY_CTRL_B;}
	    else if(CaseInsensitiveMatch(Cat,"TANK")){Categories |= 1<<UNIT_CATEGORY_TANK;}
	    else if(CaseInsensitiveMatch(Cat,"WEAPON")){Categories |= 1<<UNIT_CATEGORY_WEAPON;}
	    else if(CaseInsensitiveMatch(Cat,"LEVEL1")){Categories |= 1<<UNIT_CATEGORY_LEVEL1;}
	    else if(CaseInsensitiveMatch(Cat,"LEVEL3")){Categories |= 1<<UNIT_CATEGORY_LEVEL3;}
	    else if(CaseInsensitiveMatch(Cat,"SHIP")){Categories |= 1<<UNIT_CATEGORY_SHIP;}
	    else if(CaseInsensitiveMatch(Cat,"CTRL_W")){Categories |= 1<<UNIT_CATEGORY_CTRL_W;}
	    else if(CaseInsensitiveMatch(Cat,"SPECIAL")){Categories |= 1<<UNIT_CATEGORY_SPECIAL;}
	    else if(CaseInsensitiveMatch(Cat,"SONAR")){Categories |= 1<<UNIT_CATEGORY_SONAR;}
	    else if(CaseInsensitiveMatch(Cat,"CORE")){Categories |= 1<<UNIT_CATEGORY_CORE;}
	    else if(CaseInsensitiveMatch(Cat,"LEVEL10")){Categories |= 1<<UNIT_CATEGORY_LEVEL10;}
	    else if(CaseInsensitiveMatch(Cat,"CTRL_C")){Categories |= 1<<UNIT_CATEGORY_CTRL_C;}
	    else if(CaseInsensitiveMatch(Cat,"ENERGY")){Categories |= 1<<UNIT_CATEGORY_ENERGY;}
	    else if(CaseInsensitiveMatch(Cat,"METAL")){Categories |= 1<<UNIT_CATEGORY_METAL;}
	    else if(CaseInsensitiveMatch(Cat,"PARAL")){Categories |= 1<<UNIT_CATEGORY_PARAL;}
	    else if(CaseInsensitiveMatch(Cat,"COMMANDER")){Categories |= 1<<UNIT_CATEGORY_COMMANDER;}
	    else if(CaseInsensitiveMatch(Cat,"CARRY")){Categories |= 1<<UNIT_CATEGORY_CARRY;}
	    else if(CaseInsensitiveMatch(Cat,"CTRL_F")){Categories |= 1<<UNIT_CATEGORY_CTRL_F;}
	    else if(CaseInsensitiveMatch(Cat,"PLANT")){Categories |= 1<<UNIT_CATEGORY_PLANT;}
	    else if(CaseInsensitiveMatch(Cat,"RAD")){Categories |= 1<<UNIT_CATEGORY_RAD;}
	    else if(CaseInsensitiveMatch(Cat,"TORP")){Categories |= 1<<UNIT_CATEGORY_TORP;}
	    else if(CaseInsensitiveMatch(Cat,"CTRL_V")){Categories |= 1<<UNIT_CATEGORY_CTRL_V;}
	    else if(CaseInsensitiveMatch(Cat,"VTOL")){Categories |= 1<<UNIT_CATEGORY_VTOL;}
	    else if(CaseInsensitiveMatch(Cat,"STRATEGIC")){Categories |= 1<<UNIT_CATEGORY_STRATEGIC;}
	    else if(CaseInsensitiveMatch(Cat,"REPAIRPAD")){Categories |= (int64_t)1<<UNIT_CATEGORY_REPAIR_PAD;}
	    else if(CaseInsensitiveMatch(Cat,"JAM")){Categories |= (int64_t)1<<UNIT_CATEGORY_JAM;}
	    else if(CaseInsensitiveMatch(Cat,"DEFENSIVE")){Categories |= (int64_t)1<<UNIT_CATEGORY_DEFENSIVE;}
	    else if(CaseInsensitiveMatch(Cat,"ANTISUB")){Categories |= (int64_t)1<<UNIT_CATEGORY_ANTI_SUB;}
	    else if(CaseInsensitiveMatch(Cat,"STEALTH")){Categories |= (int64_t)1<<UNIT_CATEGORY_STEALTH;}
	    else if(CaseInsensitiveMatch(Cat,"BOMB")){Categories |= (int64_t)1<<UNIT_CATEGORY_BOMB;}
	    else if(CaseInsensitiveMatch(Cat,"PHIB")){Categories |= (int64_t)1<<UNIT_CATEGORY_PHIB;}
	    else if(CaseInsensitiveMatch(Cat,"UNDERWATER")){Categories |= (int64_t)1<<UNIT_CATEGORY_UNDERWATER;}
	    else if(CaseInsensitiveMatch(Cat,"KAMIKAZE")){Categories |= (int64_t)1<<UNIT_CATEGORY_KAMIKAZE;}		
	    else
	    {
		LogDebug("Unknown category %s encountered while parsing unit info file Category",Cat);
	    }
	    if(!done)
		*Value=' ';
	    Value++;
	}
	return Categories;
    }
    UnitSide GetSide()
    {
	char * Value = GetString("Side");
	if(!Value)
	    return SIDE_UNKNOWN;
	if(CaseInsensitiveMatch(Value,"ARM"))
	    return SIDE_ARM;
	else if(CaseInsensitiveMatch(Value,"CORE"))
	    return SIDE_CORE;
	else
	    LogDebug("Unknown side Designation in unit info: %s",Value);
	return SIDE_UNKNOWN;
    }
};

void LoadFBIFileFromBuffer(UnitDetails * UnitDeets, char * buffer, MemoryArena * GameArena)
{
    //TODO(Christof): Bounds checking?
    //[UNITINFO]
    //{
    //Name=Value;
    //....
    //}
    char * Start = strstr(buffer,"[UNITINFO]");
    if(!Start)
    {
	LogError("Could not find unit info marker in FBI:\n%s",buffer);
	return;
    }
    Start += 10;
    while(*Start==' ' || *Start =='\t' || *Start == '\r' || *Start == '\n')
    {
	Start++;
    }
    if(*Start!='{')
    {
	LogError("Malformed FBI file, found %c, expected {",*Start);
	return;
    }
    Start++;
    while(*Start==' ' || *Start =='\t' || *Start == '\r' || *Start == '\n')
    {
	Start++;
    }
    while(*Start!='}')
    {
	if(UnitDeets->DetailsSize >= MAX_UNIT_DETAILS)
	{
	    LogError("More than %d unit details",MAX_UNIT_DETAILS);
	    return;
	}
	char * Name=Start;
	while(*++Start != '=');
	*Start = 0;
	char * Value = Start+1;
	while(*++Start!=';');
	*Start++ = 0;

	UnitKeyValue temp;
	int length=(int)strlen(Name)+1;
	temp.Name=PushArray(GameArena,length,char);
	memcpy(temp.Name,Name,length);

	length=(int)strlen(Value)+1;
	temp.Value=PushArray(GameArena,length,char);
	memcpy(temp.Value,Value,length);

	UnitDeets->Details[UnitDeets->DetailsSize++]=temp;
	
	while(*Start==' ' || *Start =='\t' || *Start == '\r' || *Start == '\n')
	{
	    Start++;
	}
    }
}
