
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
	int32_t CodeSize;
	int32_t NumberOfStatics;
	int32_t Unknown_2; /* Always seems to be 0 */
	int32_t OffsetToScriptCodeIndexArray;
	int32_t OffsetToScriptNameOffsetArray;
	int32_t OffsetToPieceNameOffsetArray;
	int32_t OffsetToScriptCode;
	int32_t OffsetToScriptNames; /* Always seems to point to first script name */
};


#pragma pack(pop)

bool32 LoadUnitScriptFromBuffer(UnitScript * Script, uint8_t * Buffer, MemoryArena * GameArena)
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


    Script->ScriptDataSize = Header->CodeSize * 4;
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
const int UNIT_SCRIPT_MAX_LOCAL_VARS = 64;

enum CobCommands
{
    CMD_RAND = 0x10041000,//00 10 04 10
    CMD_LESSTHAN = 0x10051000, //00 10 05 10
    CMD_PUSHCONST = 0x10021001,//01 10 02 10	Put Constant dword onto stack
    CMD_PUSHVAR = 0x10021002,//02 10 02 10	Put local var onto stack
    CMD_CREATELCLVAR = 0x10022000,//00 20 02 10 - create space for a local variable
    CMD_SETLCLVAR = 0x10023002, //02 30 02 10	Set local var to value
    CMD_SUB = 0x10032000, //00 20 03 10 - subtract
    CMD_LESSEQUAL= 0x10052000, //00 20 05 10	VAL1 <= VAL2
    CMD_MULT = 0x10033000, //00 30 03 10	Multiply
    CMD_JMP = 0x10064000, //00 40 06 10	Jump
    CMD_EQUAL = 0x10055000, //00 50 05 10	binary equal compare
    CMD_RET = 0x10065000, //00 50 06 10	Return from function
    CMD_NEQ = 0x10056000, //00 60 05 10	!=
    CMD_IF = 0x10066000, //00 60 06 10	If
    CMD_NOT = 0x1005a000, //00 A0 05 10	NOT
    CMD_OR = 0x10036010, //10 60 03 10	Bitwise OR
    CMD_GET_UNITVAL = 0x10042000, //Get the unit value (specified by the number on the stack, i have nfi what those nums mean),
    CMD_SET_UNITVAL = 0x10082000,// as get, except it sets
    CMD_CALL_SCRIPT = 0x10063000 ,//00 30 06 10	Call-script
    CMD_OBJ_HIDE =0x10006000 ,//00 60 00 10	Hide Object
    CMD_OBJ_SHOW = 0x10005000 ,//00 50 00 10 	Show Object
    CMD_DONT_CACHE =0x10008000 ,//00 80 00 10	Don't-cache - must remember to add in displists and this means DONT use one, as the piece will need to be animated in some way (tex, movement etc.)
    CMD_DONT_SHADE = 0x1000e000,//00 E0 00 10	Don't-shade - currently does nothing, because im nopt entirely sure what it means
    CMD_START_SCRIPT = 0x10061000,//00 10 06 10	Start-script
    CMD_SLEEP = 0x10013000 ,//00 30 01 10	Sleep
    CMD_MOVE3D = 0x10001000 ,//00 10 00 10	Move object
    CMD_WAIT_TURN = 0x10011000 ,//00 10 01 10	Wait-for-turn
    CMD_TURN3D = 0x10002000 ,//00 20 00 10	Turn Object
    CMD_WAIT_MOVE = 0x10012000 ,//00 20 01 10	Wait-for-move
    CMD_SET_STATIC_VAR = 0x10023004, //04 30 02 10	Set a static-var equal to a value
    CMD_PUSH_STATIC_VAR = 0x10021004, //04 10 02 10	Put Static Var on Stack
    CMD_MOVE_NOW = 0x1000B000,
    CMD_TURN_NOW = 0x1000C000,
    CMD_CALL_SCRIPT2 = 0x10062000,//this is from TASpring
    CMD_SPIN_OBJ = 0x10003000 ,//00 30 00 10	Spin Object
    CMD_STOP_SPIN = 0x10004000,//10004000  stop_spin
    CMD_SIGNAL = 0x10067000 ,//10067000 signal             0
    CMD_SET_SIGNAL_MASK = 0x10068000 //10068000 set_signal_mask    0

};

enum Block
{
    BLOCK_NOT_BLOCKED,
    BLOCK_MOVE,
    BLOCK_TURN,
    BLOCK_SLEEP
};

struct ScriptState
{
    int StackSize;
    int32_t Stack[UNIT_SCRIPT_MAX_STACK_SIZE];
    int ProgramCounter;
    Block BlockedOn;
    int BlockTime;//NOTE(Christof): this is in milliseconds
    int BlockedOnPiece;
    int BlockedOnAxis;
    struct Object3dTransformationDetails * TransformationDetails;
};

bool32 RunScript(UnitScript * Script, int ScriptNumber, ScriptState * State)
{
    
    return 0;
}
