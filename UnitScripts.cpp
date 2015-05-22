
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
    BLOCK_SLEEP,
    BLOCK_DONE,
    BLOCK_SCRIPT
};

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
    struct Object3dTransformationDetails * TransformationDetails;
};

inline void PushStack(ScriptState * State, int32_t Value)
{
    if(State->StackSize >= UNIT_SCRIPT_MAX_STACK_SIZE)
    {
	LogError("%d items on the stack",State->StackSize);
	return;
    }
    State->Stack[State->StackSize++] = Value;
}

inline int32_t PopStack(ScriptState * State)
{
    if(State->StackSize <= 0)
    {
	LogError("No more items on the stack");
	return -1;
    }
    return State->Stack[--State->StackSize];
}

int32_t MILLISECONDS_PER_FRAME = 1000/60;

Object3dTransformationDetails * FindTransformationForPiece(Object3d * Object, Object3dTransformationDetails * TransformationDetails, char * PieceName)
{
    if(CaseInsensitiveMatch(Object->Name,PieceName))
	return TransformationDetails;

    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Object3dTransformationDetails * res = FindTransformationForPiece(&Object->Children[i], &TransformationDetails->Children[i], PieceName);
	if(res)
	    return res;
    }

    return 0;
}

inline int32_t PostData(UnitScript * Script,ScriptState * State)
{
    return Script->ScriptData[State->ProgramCounter++];
}


int GetScriptNumberForFunction(UnitScript * Script, const char * FunctionName)
{
    for(int i=0;i<Script->NumberOfFunctions;i++)
    {
	if(CaseInsensitiveMatch(Script->FunctionNames[i],FunctionName))
	   return i;
    }
    return -1;
}
const float COB_LINEAR_CONSTANT = 168340*60;
const float COB_ANGULAR_CONSTANT = 182.044444f*180.0*60/PI;

void RunScript(UnitScript * Script, ScriptState * State, Object3d * Object)
{
    if(State->ScriptNumber <0)
	return;
    //TODO(Christof): Limit time in a script
    switch(State->BlockedOn)
    {
    case BLOCK_SCRIPT:
	//NOTE(christof): the called script will wake us on return (and kill by signal I guess?)
	break;
    case BLOCK_DONE:
	return;
    case BLOCK_SLEEP:
	State->BlockTime -= MILLISECONDS_PER_FRAME;
	if(State->BlockTime < 0)
	    State->BlockedOn = BLOCK_NOT_BLOCKED;
	else
	    return;
	break;
    case BLOCK_MOVE:
    {
	char * PieceName = Script->PieceNames[State->BlockedOnPiece];
	Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	if(!PieceTransform)
	{
	    LogError("Could not find piece %s while waiting for move",PieceName);
	    return;
	}
	else
	{
	    if(PieceTransform->MovementTarget[State->BlockedOnAxis].Speed == 0)
	    {
		State->BlockedOn=BLOCK_NOT_BLOCKED;
	    }
	    else
	    {
		return;
	    }
	}
    }
    break;
    case BLOCK_TURN:
    {
	char * PieceName = Script->PieceNames[State->BlockedOnPiece];
	Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	if(!PieceTransform)
	{
	    LogError("Could not find piece %s while waiting for move",PieceName);
	    return;
	}
	else
	{
	    if(PieceTransform->RotationTarget[State->BlockedOnAxis].Speed == 0)
	    {
		State->BlockedOn=BLOCK_NOT_BLOCKED;
	    }
	    else
	    {
		return;
	    }
	}
    }
    case BLOCK_NOT_BLOCKED:
	break;
    }
    if(State->ProgramCounter == 0)
    {
	State->ProgramCounter = Script->FunctionOffsets[State->ScriptNumber];
	LogDebug("Set Program counter to %d",State->ProgramCounter);
    }

    //NOTE(Christof): in cob instructions doc, top of the stack is at the BOTTOM of the list
    while(1)
    {
	//TODO(Christof): Implement functions
	switch(Script->ScriptData[State->ProgramCounter++])
	{
	case COB_MOVE:
	{
	    int32_t Target = PopStack(State);
	    int32_t Speed = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->MovementTarget[Axis].Destination = Target/COB_LINEAR_CONSTANT;
	    PieceTransform->MovementTarget[Axis].Speed = Speed/COB_LINEAR_CONSTANT;
	}
	break;
	case COB_TURN:
	{
	    int32_t Target = PopStack(State);
	    int32_t Speed = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->RotationTarget[Axis].Heading = Target/COB_ANGULAR_CONSTANT;
	    PieceTransform->RotationTarget[Axis].Speed = Speed/COB_ANGULAR_CONSTANT;
	}
	break;
	case COB_SPIN:
	{
	    int32_t Speed = PopStack(State);
	    int32_t Acceleration = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	    //NOTE(Christof): if Acceleration is 0 Spin change is immediate

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    if(Acceleration == 0)
	    {
		PieceTransform->Spin[Axis]=Speed/COB_ANGULAR_CONSTANT;
	    }
	    else
	    {
		PieceTransform->SpinTarget[Axis].Acceleration = Acceleration/COB_ANGULAR_CONSTANT;
		PieceTransform->SpinTarget[Axis].Speed = Speed/COB_ANGULAR_CONSTANT;
	    }
	}
	break;
	case COB_SPIN_STOP:
	{
	    int32_t Speed = PopStack(State);
	    int32_t Acceleration = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	    if(Speed !=0)
	    {
		LogWarning("Spin stop called with non zero speed %d?",Speed);
	    }
	    //NOTE(Christof): if Acceleration is 0 Spin change is immediate

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    if(Acceleration == 0)
	    {
		PieceTransform->Spin[Axis]=0;
	    }
	    else
	    {
		PieceTransform->SpinTarget[Axis].Acceleration = Acceleration/COB_ANGULAR_CONSTANT;
		PieceTransform->SpinTarget[Axis].Speed = 0;
	    }

	}
	break;
	case COB_SHOW:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_HIDE:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_CACHE:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_DONT_CACHE:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_MOVE_NOW:
	{
	    int32_t Target = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	}
	break;
	case COB_TURN_NOW:
	{
	    int32_t Target = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	}
	break;
	case COB_SHADE:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_DONT_SHADE:
	{
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_EMIT_SPECIAL_EFFECTS:
	{
	    int32_t EffectType = PopStack(State);
	    int32_t Piece = PostData(Script,State);
	}
	break;
	case COB_WAIT_FOR_TURN:
	{
	    int32_t Piece = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	    
	}
	break;
	case COB_WAIT_FOR_MOVE:
	{
	    int32_t Piece = PostData(Script,State);
	    int32_t Axis = PostData(Script,State);
	}
	break;
	case COB_SLEEP:
	{
	    int32_t Millis = PopStack(State);
	}
	break;
	case COB_CREATE_LOCAL_VARIABLE:
	    //NOTE(Christof): the set should probably be unecessary, but best to be sure
	    State->LocalVariables[State->NumberOfLocalVariables++]=0;
	break;
	case COB_STACK_FLUSH:
	    LogDebug("Flushing stack in %s",Script->FunctionNames[State->ScriptNumber]);
	    State->StackSize=0;
	    break;
	case COB_PUSH_CONSTANT:
	    PushStack(State, PostData(Script,State));
	    break;
	case COB_PUSH_LOCAL_VARIABLE:
	    PushStack(State, State->LocalVariables[PostData(Script,State)]);
	    break;
	case COB_PUSH_STATIC_VARIABLE:
	    PushStack(State, State->StaticVariables[PostData(Script,State)]);
	    break;
	case COB_POP_LOCAL_VARIABLE:
	    State->LocalVariables[PostData(Script,State)] = PopStack(State);
	    break;
	case COB_POP_STATIC_VARIABLE:
	    State->StaticVariables[PostData(Script,State)] = PopStack(State);
	    break;
	case COB_ADD:
	    PushStack(State, PopStack(State)+PopStack(State));
	    break;
	case COB_SUBTRACT:
	    PushStack(State, PopStack(State)-PopStack(State));
	    break;
	case COB_MULTIPLY:
	    PushStack(State, PopStack(State) * PopStack(State));
	    break;
	case COB_DIVIDE:
	    PushStack(State, PopStack(State) / PopStack(State));
	    break;
	case COB_BITWISE_AND:
	    PushStack(State, PopStack(State) & PopStack(State));
	    break;
	case COB_BITWISE_OR:
	    PushStack(State, PopStack(State) | PopStack(State));
	    break;
	case COB_BITWISE_XOR:
	    PushStack(State, PopStack(State) ^ PopStack(State));
	    break;
	case COB_BITWISE_NOT:
	    PushStack(State, ~PopStack(State));
	    break;
	case COB_RANDOM:
	{
	    int32_t LowerBound = PopStack(State);
	    int32_t UpperBound = PopStack(State);
	    int32_t Result = UpperBound;
	    PushStack(State, Result);
	}
	break;
	case COB_GET_UNIT_VALUE:
	{
	    int32_t GetValue = PopStack(State);
	    int32_t Value = 1;
	    PushStack(State, Value);
	}
	break;
	case COB_GET_VALUE:
	{
	    int32_t GetValue = PopStack(State);
	    int32_t Value = 1;
	    PushStack(State, Value);   
	}
	break;
	case COB_UNKNOWN5:
	case COB_UNKNOWN6:
	    //NOTE(Christof): these seem to just push 0 onto the stack?
	    LogDebug("Pushing 0 onto the stack for unknown5/6");
	    PushStack(State, 0);
	    break;
	case COB_LESS_THAN:
	    PushStack(State, PopStack(State) < PopStack(State));
	    break;
	case COB_LESS_THAN_OR_EQUAL:
	    PushStack(State, PopStack(State) <= PopStack(State));
	    break;
	case COB_GREATER_THAN:
	    PushStack(State, PopStack(State) > PopStack(State));
	    break;
	case COB_GREATER_THAN_OR_EQUAL:
	    PushStack(State, PopStack(State) >= PopStack(State));
	    break;
	case COB_EQUAL:
	    PushStack(State, PopStack(State) == PopStack(State));
	    break;
	case COB_NOT_EQUAL:
	    PushStack(State, PopStack(State) != PopStack(State));
	    break;
	case COB_AND:
	{
	    int32_t Val1 = PopStack(State);
	    int32_t Val2 = PopStack(State);
	    PushStack(State, Val1 && Val2);
	}
	break;
	case COB_OR:
	{
	    int32_t Val1 = PopStack(State);
	    int32_t Val2 = PopStack(State);
	    PushStack(State, Val1 || Val2);
	}
	break;
	case COB_XOR:
	{
	    int32_t Val1 = PopStack(State);
	    int32_t Val2 = PopStack(State);
	    PushStack(State, (Val2 || Val1) && (!(Val2 && Val1)));
	}
	break;
	case COB_NOT:
	    PushStack(State,!PopStack(State));
	    break;
	case COB_START_SCRIPT:
	{
	    int32_t FunctionNumber = PostData(Script,State);
	    int32_t NumberOfArguments = PostData(Script,State);
	    //TODO(Christof): Figure out how to do running more than a single script at a time (including CALLING functions)
	    for(int i=0;i<NumberOfArguments;i++)
	    {
		int Argumenti = PopStack(State);
		//Some of the docs suggest the stack should maybe be empty after this?
	    }
	}
	break;
	case COB_CALL_SCRIPT:
	{
	    int32_t FunctionNumber = PostData(Script,State);
	    int32_t NumberOfArguments = PostData(Script,State);
	    //TODO(Christof): Figure out how to do running more than a single script at a time (including CALLING functions)
	    for(int i=0;i<NumberOfArguments;i++)
	    {
		int Argumenti = PopStack(State);
		//Some of the docs suggest the stack should maybe be empty after this?
	    }
	    LogError("Script call to %s attempted, not currently supported", Script->FunctionNames[FunctionNumber]);
	}
	break;
	case COB_JUMP:
	    //NOTE(Christof): this needs testing/TA decompile, seems likely this is unconditional, docs are divided
	    //NOTE(Chritof): this may not need to remove a value from the stack, again docs unclear, much less obvious correct choice
	    PopStack(State);//maybe if on this?
	    State->ProgramCounter = PostData(Script,State);
	    break;
	case COB_RETURN:
	{
	    State->BlockedOn=BLOCK_DONE;
	    if(State->ReturnTo)
	    {
		PushStack(State->ReturnTo,PopStack(State));
		State->ReturnTo->BlockedOn = BLOCK_NOT_BLOCKED;
	    }
	    return;
	}
	case COB_JUMP_IF_FALSE:
	{
	    int32_t NewProgramCounter = PostData(Script,State);
	    if(!PopStack(State))
	    {
		State->ProgramCounter = NewProgramCounter;
	    }
	    break;
	}
	case COB_SIGNAL:
	{
	    int32_t Signal = PopStack(State);
	    LogError("We do not currently handle signals");
	}
	break;
	case COB_SET_SIGNAL_MASK:
	{
	    //TODO(Christof): Determine the exact usage of this, does is straight set, flip bits, bitwise or?
	    State->SignalMask=PopStack(State);
	}
	break;
	case COB_EXPLODE:
	{
	    int32_t ExplodeType = PopStack(State);
	    int32_t PieceNumber = PostData(Script,State);
	}
	break;
	case COB_SET_UNIT_VALUE:
	{
	    int32_t UnitVariableNumber = PopStack(State);
	    int32_t Value = PopStack(State);
	}
	break;
	case COB_ATTACH_UNIT:
	{
	    int32_t UnitID = PopStack(State);
	    int32_t PieceNumber = PopStack(State);
	    int32_t Unknown = PopStack(State);
	}
	break;
	case COB_DROP_UNIT:
	{
	    int32_t UnitID = PopStack(State);
	}
	break;
	}
    }
}
