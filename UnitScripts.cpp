
const int SCRIPT_NAME_STORAGE_SIZE=64;
struct UnitScript
{
    int NumberOfFunctions;
    int NumberOfPieces;
    char ** FunctionNames;
    char ** PieceNames;
    int ScriptDataSize;
    int32_t * ScriptData;
    int32_t * FunctionOffsets;
    int32_t NumberOfStatics;
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
    Script->NumberOfFunctions = Header->NumberOfScripts;

    Script->FunctionNames = PushArray(GameArena, Script->NumberOfFunctions, char*);
    Script->FunctionNames[0] = PushArray(GameArena, Script->NumberOfFunctions * SCRIPT_NAME_STORAGE_SIZE, char);

    for(int i=1;i<Script->NumberOfFunctions;i++)
    {
	Script->FunctionNames[i] = Script->FunctionNames[0] + SCRIPT_NAME_STORAGE_SIZE*i;
    }
    int32_t * NameOffsetArray = (int32_t *)(Buffer + Header->OffsetToScriptNameOffsetArray);
    for(int i=0;i<Script->NumberOfFunctions;i++)
    {
	int Length = (int)strlen((char*)Buffer + NameOffsetArray[i]);
	Length = Length<SCRIPT_NAME_STORAGE_SIZE?Length:SCRIPT_NAME_STORAGE_SIZE-1;
	memcpy(Script->FunctionNames[i],Buffer + NameOffsetArray[i], Length);
	Script->FunctionNames[i][Length]=0;
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
    Script->ScriptData = PushArray(GameArena, Script->ScriptDataSize, int32_t);
    Script->FunctionOffsets = PushArray(GameArena, Script->NumberOfFunctions, int32_t);
    int32_t * ScriptOffsetArray = (int32_t *)(Buffer + Header->OffsetToScriptCodeIndexArray);
    for(int i=0;i<Script->NumberOfFunctions;i++)
    {
	Script->FunctionOffsets[i] = ScriptOffsetArray[i];
    }
    memcpy(Script->ScriptData, Buffer+Header->OffsetToScriptCode, Script->ScriptDataSize);
    
    return 1;
}

const int UNIT_SCRIPT_MAX_STACK_SIZE = 128;
const int UNIT_SCRIPT_MAX_LOCAL_VARS = 64;

enum CobCommands
{
    //Piece Operations:
    COB_MOVE      = 0x10001000,
    COB_TURN      = 0x10002000,
    COB_SPIN      = 0x10003000,
    COB_SPIN_STOP = 0x10004000,
    COB_SHOW      = 0x10005000,
    COB_HIDE = 0x10006000,
    COB_CACHE = 0x10007000,
    COB_DONT_CACHE = 0x10008000,
    COB_UNKNOWN1 = 0x10009000,
    COB_UNKNOWN2 = 0x1000A000,
    COB_MOVE_NOW = 0x1000B000,
    COB_TURN_NOW = 0x1000C000,
    COB_SHADE = 0x1000D000,
    COB_DONT_SHADE = 0x1000E000,
    COB_EMIT_SPECIAL_EFFECTS = 0x1000F000,

    //Wait Instructions:
    COB_WAIT_FOR_TURN = 0x10011000,
    COB_WAIT_FOR_MOVE = 0x10012000,
    COB_SLEEP = 0x10013000,

    //Memory and Stack Instructions:
    COB_UNKNOW3 = 0x10021000,
    COB_CREATE_LOCAL_VARIABLE = 0x10022000,
    COB_UNKNOWN3 = 0x10023000,
    COB_STACK_FLUSH = 10024000,
    COB_PUSH_CONSTANT = 0x10021001,
    COB_PUSH_LOCAL_VARIABLE = 0x10021002,
    COB_PUSH_STATIC_VARIABLE = 0x10021004,
    COB_POP_LOCAL_VARIABLE = 0x10023002,
    COB_POP_STATIC_VARIABLE = 0x10023004,

    //Bitwise and Arithmetic Operations
    COB_ADD = 0x10031000,
    COB_SUBTRACT = 0x10032000,
    COB_MULTIPLY = 0x10033000,
    COB_DIVIDE = 0x10034000,
    COB_BITWISE_AND = 0x10035000,
    COB_BITWISE_OR = 0x10036000,
    COB_BITWISE_XOR = 0x10037000,
    COB_BITWISE_NOT = 0x10038000,

    //Value Getting Functions
    COB_RANDOM = 0x10041000,
    COB_GET_UNIT_VALUE = 0x10042000,
    COB_GET_VALUE = 0x10043000,//NOTE(Christof): it is unclear what the difference between the two get functions are at this stage
    COB_UNKNOWN5 = 0x10044000,//NOTE(Christof): these two unknowns apparently just push 0 onto the stack (seems unlikely there would be two opcodes if they aren't in some way distinct)
    COB_UNKNOWN6 = 0x10045000,

    //Logical Comparison Operations
    COB_LESS_THAN = 0x10051000,
    COB_LESS_THAN_OR_EQUAL = 0x10052000,
    COB_GREATER_THAN = 0x10053000,
    COB_GREATER_THAN_OR_EQUAL = 0x10054000,
    COB_EQUAL = 0x10055000,
    COB_NOT_EQUAL = 0x10056000,
    COB_AND = 0x10057000,
    COB_OR = 0x10058000,
    COB_XOR = 0x10059000,
    COB_NOT = 0x1005A000,

    //Flow Control:
    COB_START_SCRIPT = 0x10061000,
    COB_CALL_SCRIPT = 0x10062000,
    COB_UNKNOWN7 = 0x10063000,
    COB_JUMP = 0x10064000,
    COB_RETURN = 0x10065000,
    COB_JUMP_IF_FALSE = 0x10066000,
    COB_SIGNAL = 0x10067000,
    COB_SET_SIGNAL_MASK = 0x10068000,

    //Explode:
    COB_EXPLODE = 0x10071000,

    //Set Unit Values:
    COB_SET_UNIT_VALUE = 0x10082000,
    COB_ATTACH_UNIT = 0x10083000,
    COB_DROP_UNIT = 0x10084000
    
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
    //TODO(Christof): determine if stack can contain floats? some docs seem to indicate they should?
    int StackSize;
    int32_t Stack[UNIT_SCRIPT_MAX_STACK_SIZE];
    int NumberOfLocalVariables;
    int32_t LocalVariables[UNIT_SCRIPT_MAX_STACK_SIZE];//Parameters at beginning
    int NumberOfStaticVariables;
    int32_t * StaticVariables;
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
