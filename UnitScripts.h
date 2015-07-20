
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

enum Block
{
    BLOCK_NOT_BLOCKED,
    BLOCK_MOVE,
    BLOCK_TURN,
    BLOCK_SLEEP,
    BLOCK_DONE,
    BLOCK_SCRIPT
};

const int UNIT_SCRIPT_MAX_STACK_SIZE = 128;
const int UNIT_SCRIPT_MAX_LOCAL_VARS = 64;

const int MAX_INSTRUCTIONS_PER_FRAME = 2000;
struct ScriptState
{
    //TODO(Christof): determine if stack can contain floats? some docs seem to indicate they should?
    int StackSize;
    int32_t Stack[UNIT_SCRIPT_MAX_STACK_SIZE];
    int NumberOfLocalVariables;
    int32_t LocalVariables[UNIT_SCRIPT_MAX_STACK_SIZE];//NOTE(Christof): function parameters go at the beginning
    int NumberOfStaticVariables;
    int32_t * StaticVariables;
    int ProgramCounter;
    Block BlockedOn;
    int BlockTime;//NOTE(Christof): this is in milliseconds
    int BlockedOnPiece;
    int BlockedOnAxis;
    uint32_t SignalMask;
    ScriptState * ReturnTo;
    int ScriptNumber;
    int NumberOfParameters;
    struct Object3dTransformationDetails * TransformationDetails;
    int CurrentInstructionCount;
};

const int SCRIPT_POOL_SIZE = 64;
struct ScriptStatePool
{
    ScriptState Scripts[SCRIPT_POOL_SIZE];
    int NumberOfScripts;
};
