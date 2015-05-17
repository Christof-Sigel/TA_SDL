

const int SCRIPT_NAME_STORAGE_SIZE=64;
struct UnitScript
{
    int NumberOfScripts;
    int NumberOfPieces;
    char ** ScriptNames;
    char ** PieceNames;
    int ScriptDataSize;
    uint8_t * ScriptData;
    uint8_t ** Scripts;
};


#pragma pack(push,1)
struct FILE_CobHeader
{
	int32_t VersionSignature;
	int32_t NumberOfScripts;
	int32_t NumberOfPieces;
	int32_t Unknown_0;
	int32_t Unknown_1;
	int32_t Unknown_2; /* Always seems to be 0 */
	int32_t OffsetToScriptCodeIndexArray;
	int32_t OffsetToScriptNameOffsetArray;
	int32_t OffsetToPieceNameOffsetArray;
	int32_t OffsetToScriptCode;
	int32_t Unknown_3; /* Always seems to point to first script name */
};


#pragma pack(pop)

bool32 LoadUnitScriptFromBuffer(UnitScript * Script, uint8_t * Buffer, int BufferSize, MemoryArena * GameArena)
{
    FILE_CobHeader * Header = (FILE_CobHeader *)Buffer;
    Script->NumberOfPieces = Header->NumberOfPieces;
    Script->NumberOfScripts = Header->NumberOfScripts;

    Script->ScriptNames = PushArray(GameArena, Script->NumberOfScripts, char*);
    Script->ScriptNames[0] = PushArray(GameArena, Script->NumberOfScripts * SCRIPT_NAME_STORAGE_SIZE, char);

    for(int i=1;i<Script->NumberOfScripts;i++)
    {
	Script->ScriptNames[i] = Script->ScriptNames[0] + SCRIPT_NAME_STORAGE_SIZE*i;
    }
    int32_t * NameOffsetArray = (int32_t *)(Buffer + Header->OffsetToScriptNameOffsetArray);
    for(int i=0;i<Script->NumberOfScripts;i++)
    {
	int Length = (int)strlen((char*)Buffer + NameOffsetArray[i]);
	Length= Length<SCRIPT_NAME_STORAGE_SIZE?Length:SCRIPT_NAME_STORAGE_SIZE-1;
	memcpy(Script->ScriptNames[i],Buffer + NameOffsetArray[i], Length);
	Script->ScriptNames[i][Length]=0;
    }
    
    Script->PieceNames = PushArray(GameArena, Script->NumberOfPieces, char*);
    Script->PieceNames[0] = PushArray(GameArena, Script->NumberOfPieces * SCRIPT_NAME_STORAGE_SIZE, char);
    for(int i=1;i<Script->NumberOfPieces;i++)
    {
	Script->PieceNames[i] = Script->PieceNames[0] + SCRIPT_NAME_STORAGE_SIZE*i; 
    }
    NameOffsetArray = (int32_t *)(Buffer + Header->OffsetToPieceNameOffsetArray);
    for(int i=0;i<Script->NumberOfPieces;i++)
    {
	int Length = (int)strlen((char*)Buffer + NameOffsetArray[i]);
	Length= Length<SCRIPT_NAME_STORAGE_SIZE?Length:SCRIPT_NAME_STORAGE_SIZE-1;
	memcpy(Script->PieceNames[i],Buffer + NameOffsetArray[i], Length);
	Script->PieceNames[i][Length]=0;
    }


    Script->ScriptDataSize = BufferSize - Header->OffsetToScriptCode;
    Script->ScriptData = PushArray(GameArena, Script->ScriptDataSize, uint8_t);
    Script->Scripts = PushArray(GameArena, Script->NumberOfScripts, uint8_t *);
    int32_t * ScriptOffsetArray = (int32_t *)(Buffer + Header->OffsetToScriptCodeIndexArray);
    for(int i=0;i<Script->NumberOfScripts;i++)
    {
	Script->Scripts[i] = Script->ScriptData+ScriptOffsetArray[i];
    }
    memcpy(Script->ScriptData, Buffer+Header->OffsetToScriptCode, Script->ScriptDataSize);
    
    return 1;
}

const int UNIT_SCRIPT_MAX_STACK_SIZE = 128;
struct ScriptState
{
    int StackSize;
    int32_t Stack[UNIT_SCRIPT_MAX_STACK_SIZE];
    int ProgramCounter;
};

bool32 RunScript(UnitScript * Script, int ScriptNumber, ScriptState * State)
{

    return 0;
}
