b32 LoadUnitScriptFromBuffer(UnitScript * Script, u8 * Buffer, MemoryArena * GameArena)
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
    s32 * NameOffsetArray = (s32  *)(Buffer + Header->OffsetToScriptNameOffsetArray);
    for(int i=0;i<Script->NumberOfFunctions;i++)
    {
	int Length = (int)strlen((char*)Buffer + NameOffsetArray[i]);
	Length = Length<SCRIPT_NAME_STORAGE_SIZE?Length:SCRIPT_NAME_STORAGE_SIZE-1;
	memcpy(Script->FunctionNames[i],Buffer + NameOffsetArray[i], Length);
	Script->FunctionNames[i][Length]=0;
    }
    
    Script->PieceNames = PushArray(GameArena, Script->NumberOfPieces, char*);
    if(Script->PieceNames)
    {
	Script->PieceNames[0] = PushArray(GameArena, Script->NumberOfPieces * SCRIPT_NAME_STORAGE_SIZE, char);
	for(int i=1;i<Script->NumberOfPieces;i++)
	{
	    Script->PieceNames[i] = Script->PieceNames[0] + SCRIPT_NAME_STORAGE_SIZE*i; 
	}
	NameOffsetArray = (s32  *)(Buffer + Header->OffsetToPieceNameOffsetArray);
	for(int i=0;i<Script->NumberOfPieces;i++)
	{
	    int Length = (int)strlen((char*)Buffer + NameOffsetArray[i]);
	    Length= Length<SCRIPT_NAME_STORAGE_SIZE?Length:SCRIPT_NAME_STORAGE_SIZE-1;
	    memcpy(Script->PieceNames[i],Buffer + NameOffsetArray[i], Length);
	    Script->PieceNames[i][Length]=0;
	}
    }
    Script->NumberOfStatics = Header->NumberOfStatics;

    Script->ScriptDataSize = Header->CodeSize * 4;
    Script->ScriptData = PushArray(GameArena, Script->ScriptDataSize, s32 );
    Script->FunctionOffsets = PushArray(GameArena, Script->NumberOfFunctions, s32 );
    s32 * ScriptOffsetArray = (s32  *)(Buffer + Header->OffsetToScriptCodeIndexArray);
    for(int i=0;i<Script->NumberOfFunctions;i++)
    {
	Script->FunctionOffsets[i] = ScriptOffsetArray[i];
    }
    memcpy(Script->ScriptData, Buffer+Header->OffsetToScriptCode, Script->ScriptDataSize);
    
    return 1;
}


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
    COB_CALL_SCRIPT2 = 0x10063000,
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




inline void PushStack(ScriptState * State, s32 Value)
{
    Assert(State->StackSize < UNIT_SCRIPT_MAX_STACK_SIZE);
    State->StackData[State->StackSize++] = Value;
}

inline s32 PopStack(ScriptState * State)
{
    Assert(State->StackSize > 0)
    return State->StackData[--State->StackSize];
}

int MILLISECONDS_PER_FRAME = 1000/60;

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

inline s32 PostData(UnitScript * Script,ScriptState * State)
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

void CompactScriptPool(ScriptStatePool * Pool)
{
    for(int i=Pool->NumberOfScripts - 2 ; i>=0;i--)
    {
	if(Pool->Scripts[i].BlockedOn == BLOCK_DONE)
	{
	    for(int j=i;j<Pool->NumberOfScripts ; j++)
	    {
		Pool->Scripts[j] =Pool->Scripts[j+1];
	    }
	    for(int k=0;k<Pool->NumberOfScripts;k++)
	    {
		if(Pool->Scripts[k].ReturnTo > &Pool->Scripts[i])
		    Pool->Scripts[k].ReturnTo--;
	    }
	    Pool->NumberOfScripts--;
	}
    }
}

void CleanUpScriptPool(ScriptStatePool * Pool)
{
    int i;
    for(i=Pool->NumberOfScripts - 1 ; i>=0;i--)
    {
	if(Pool->Scripts[i].BlockedOn != BLOCK_DONE)
	{
	    break;
	}
    }
    Pool->NumberOfScripts = i+1;
    CompactScriptPool(Pool);
}

ScriptState * AddNewScript(ScriptStatePool * Pool, UnitScript * Script, s32 NumberOfArguments, s32 * Arguments, Object3dTransformationDetails * TransformationDetails, s32 FunctionNumber, s32 SignalMask =0)
{
    if(Pool->NumberOfScripts >= SCRIPT_POOL_SIZE)
	return 0;
    ScriptState * NewState = &Pool->Scripts[Pool->NumberOfScripts++];
     *NewState = {};
    for(int i=NumberOfArguments-1;i>=0;i--)
    {
	NewState->LocalVariables[i]= Arguments[i];
    }
    NewState->NumberOfLocalVariables = NumberOfArguments;
    NewState->StaticVariables = Pool->StaticVariables;
    NewState->NumberOfStaticVariables = Script->NumberOfStatics;
    NewState->ScriptNumber = FunctionNumber;
    NewState->SignalMask = SignalMask;
    NewState->StackData = NewState->StackStorage;
   
    NewState->TransformationDetails = TransformationDetails;
    NewState->NumberOfParameters = NumberOfArguments;
    return NewState;
}

ScriptState * StartNewEntryPoint(ScriptStatePool * Pool, UnitScript * Script, const char * FunctionName, s32 NumberOfArguments, s32 * Arguments, Object3dTransformationDetails * TransformationDetails)
{
    s32 FunctionNumber =  GetScriptNumberForFunction( Script, FunctionName);
    if(FunctionNumber < 0)
	return 0;
    return AddNewScript(Pool,Script, NumberOfArguments, Arguments, TransformationDetails, FunctionNumber);
}

ScriptState * StartNewEntryPoint(ScriptStatePool * Pool, UnitScript * Script, s32 FunctionNumber, s32 NumberOfArguments, s32 * Arguments, Object3dTransformationDetails * TransformationDetails)
{
    if(FunctionNumber < 0)
	return 0;
    return AddNewScript(Pool,Script, NumberOfArguments, Arguments, TransformationDetails, FunctionNumber);
}

ScriptState * CreateNewScriptState(UnitScript * Script, ScriptState * State, ScriptStatePool * Pool)
{
    s32 FunctionNumber = PostData(Script,State);
    s32 NumberOfArguments = PostData(Script,State);
    s32 Arguments[32];
    for(int i=0;i<NumberOfArguments; i++)
	Arguments[i]=PopStack(State);
    return AddNewScript(Pool, Script, NumberOfArguments, Arguments, State->TransformationDetails, FunctionNumber,  State->SignalMask);
}

s32 RunScript(UnitScript * Script, ScriptState * State, Object3d * Object, ScriptStatePool * Pool, const s32 InstructionsToRun = MAX_INSTRUCTIONS_PER_FRAME)
{
    if(State->ScriptNumber < 0)
	return -1;
    switch(State->BlockedOn)
    {
    case BLOCK_SCRIPT:
	//NOTE(christof): the called script will wake us on return (and kill by signal I guess?)
	return -1;
    case BLOCK_DONE:
	return -1;
    case BLOCK_SLEEP:
	State->BlockTime -= MILLISECONDS_PER_FRAME;
	if(State->BlockTime < 0)
	    State->BlockedOn = BLOCK_NOT_BLOCKED;
	else
	    return -1;
	break;
    case BLOCK_MOVE:
    {
	char * PieceName = Script->PieceNames[State->BlockedOnPiece];
	Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	if(!PieceTransform)
	{
	    LogError("Could not find piece %s while waiting for move",PieceName);
	    return -1;
	}
	else
	{
	    if(PieceTransform->MovementTarget[State->BlockedOnAxis].Speed == 0)
	    {
		State->BlockedOn=BLOCK_NOT_BLOCKED;
	    }
	    else
	    {
		return -1;
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
	    return -1;

	}
	else
	{
	    if(PieceTransform->RotationTarget[State->BlockedOnAxis].Speed == 0)
	    {
		State->BlockedOn=BLOCK_NOT_BLOCKED;
	    }
	    else
	    {
		return -1;
	    } 
	}
    }
    case BLOCK_NOT_BLOCKED:
	break;
    case BLOCK_INIT:
	State->ProgramCounter = Script->FunctionOffsets[State->ScriptNumber];
	State->BlockedOn = BLOCK_NOT_BLOCKED;
	break;
    }
    
    int CurrentInstructionCount=0;

    //NOTE(Christof): in cob instructions doc, top of the stack is at the BOTTOM of the list
    for(;;)
    {
	if(++CurrentInstructionCount > InstructionsToRun)
	{
	    return -1;
	}
	switch(Script->ScriptData[State->ProgramCounter++])
	{
	case COB_MOVE:
	{
	    s32 Target = PopStack(State);
	    s32 Speed = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);
	    char * PieceName = Script->PieceNames[PieceNumber];
	    switch(Axis)
	    {
	    case TA_AXIS_X:
		Target = Target * TranslationXAxisModScript;
		break;
	    case TA_AXIS_Y:
		Target = Target * TranslationYAxisModScript;
		break;
	    case TA_AXIS_Z:
		Target = Target * TranslationZAxisModScript;
		break;
	    }

	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->MovementTarget[Axis].Destination = Target/COB_LINEAR_CONSTANT;
	    PieceTransform->MovementTarget[Axis].Speed = Speed/COB_LINEAR_FRAME_CONSTANT;
	}
	break;
	case COB_TURN:
	{
	    s32 Target = PopStack(State);
	    s32 Speed = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);
	    switch(Axis)
	    {
	    case TA_AXIS_X:
		Target = Target * RotationXAxisMod;
		break;
	    case TA_AXIS_Y:
		Target = Target * RotationYAxisMod;
		break;
	    case TA_AXIS_Z:
		Target = Target * RotationZAxisMod;
		break;
	    }

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->RotationTarget[Axis].Heading = Target/COB_ANGULAR_CONSTANT;
	    PieceTransform->RotationTarget[Axis].Speed = Speed/COB_ANGULAR_FRAME_CONSTANT;
	}
	break;
	case COB_SPIN:
	{
	    s32 Speed = PopStack(State);
	    s32 Acceleration = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);
	    //NOTE(Christof): if Acceleration is 0 Spin change is immediate
	    switch(Axis)
	    {
	    case TA_AXIS_X:
		Speed = Speed * RotationXAxisMod;
		break;
	    case TA_AXIS_Y:
		Speed = Speed * RotationYAxisMod;
		break;
	    case TA_AXIS_Z:
		Speed = Speed * RotationZAxisMod;
		break;
	    }

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    if(Acceleration == 0)
	    {
		PieceTransform->Spin[Axis]=Speed/COB_ANGULAR_FRAME_CONSTANT/64;
	    }
	    else
	    {
		PieceTransform->SpinTarget[Axis].Acceleration = Acceleration/COB_ANGULAR_FRAME_CONSTANT;
		PieceTransform->SpinTarget[Axis].Speed = Speed/COB_ANGULAR_FRAME_CONSTANT/4;
	    }
	}
	break;
	case COB_SPIN_STOP:
	{
	    s32 Acceleration = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    if(Acceleration == 0)
	    {
		PieceTransform->Spin[Axis]=0;
	    }
	    else
	    {
		PieceTransform->SpinTarget[Axis].Acceleration = Acceleration/COB_ANGULAR_FRAME_CONSTANT/64;
		PieceTransform->SpinTarget[Axis].Speed = 0;
	    }
	}
	break;
	case COB_SHOW:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags &= ~OBJECT3D_FLAG_HIDE;
	}
	break;
	case COB_HIDE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags |= OBJECT3D_FLAG_HIDE;
	}
	break;
	case COB_CACHE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags |= OBJECT3D_FLAG_CACHE;
	}
	break;
	case COB_DONT_CACHE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags &= ~OBJECT3D_FLAG_CACHE;
	}
	break;
	case COB_MOVE_NOW:
	{
	    s32 Target = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);
	    switch(Axis)
	    {
	    case TA_AXIS_X:
		Target = Target * TranslationXAxisModScript;
		break;
	    case TA_AXIS_Y:
		Target = Target * TranslationYAxisModScript;
		break;
	    case TA_AXIS_Z:
		Target = Target * TranslationZAxisModScript;
		break;
	    }

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Movement[Axis] = Target/COB_LINEAR_CONSTANT;
	}
	break;
	case COB_TURN_NOW:
	{
	    s32 Target = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);

	    switch(Axis)
	    {
	    case TA_AXIS_X:
		Target = Target * RotationXAxisMod;
		break;
	    case TA_AXIS_Y:
		Target = Target * RotationYAxisMod;
		break;
	    case TA_AXIS_Z:
		Target = Target * RotationZAxisMod;
		break;
	    }

	    char * PieceName = Script->PieceNames[PieceNumber];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Rotation[Axis] = Target/COB_ANGULAR_CONSTANT;
	}
	break;
	case COB_SHADE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags &= ~OBJECT3D_FLAG_DONT_SHADE;
	}
	break;
	case COB_DONT_SHADE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags |= OBJECT3D_FLAG_DONT_SHADE;
	}
	break;
	case COB_EMIT_SPECIAL_EFFECTS:
	{
	    s32 EffectType = PopStack(State);
	    s32 Piece = PostData(Script,State);
	    LogWarning("Ignoring Special effect %d for %s",EffectType,Script->PieceNames[Piece]);
	}
	break;
	case COB_WAIT_FOR_TURN:
	    State->BlockedOn = BLOCK_TURN;
	    State->BlockedOnPiece = PostData(Script,State);
	    State->BlockedOnAxis = PostData(Script,State);
	    return -1;
	    break;
	case COB_WAIT_FOR_MOVE:
	    State->BlockedOn = BLOCK_MOVE;
	    State->BlockedOnPiece = PostData(Script,State);
	    State->BlockedOnAxis = PostData(Script,State);
	    return -1;
	    break;
	case COB_SLEEP:
	    State->BlockedOn = BLOCK_SLEEP;
	    State->BlockTime = PopStack(State);
	    return -1;
	case COB_CREATE_LOCAL_VARIABLE:
	    if(State->NumberOfParameters)
		State->NumberOfParameters--;
	    else
		State->NumberOfLocalVariables++;
	    break;
	case COB_STACK_FLUSH:
	    LogDebug("Flushing stack in %s",Script->FunctionNames[State->ScriptNumber]);
//	    State->StackSize=0;
	    Assert(!"NOPE");
	    break;
	case COB_PUSH_CONSTANT:
	    PushStack(State, PostData(Script,State));
	    break;
	case COB_PUSH_LOCAL_VARIABLE:
	{
	    s32 Index = PostData(Script,State);
	    if(Index>=State->NumberOfLocalVariables)
	    {
		LogError("%s Trying to push the value of local variable %d, but only %d exist, pushing 0 instead",Script->FunctionNames[State->ScriptNumber], Index, State->NumberOfLocalVariables);
		PushStack(State,0);
	    }
	    else
	    { 
		PushStack(State, State->LocalVariables[Index]);
	    }
	}
	break;
	case COB_PUSH_STATIC_VARIABLE:
	{
	    s32 Index = PostData(Script,State);
	    if(Index >= State->NumberOfStaticVariables)
	    {
	    	LogError("%s Trying to push the value of static variable %d, but only %d exist, pushing 0 instead",Script->FunctionNames[State->ScriptNumber], Index, State->NumberOfStaticVariables);
		PushStack(State, 0);
	    }
	    else
	    { 
		PushStack(State, State->StaticVariables[Index]);
	    }
	}
	break;
	case COB_POP_LOCAL_VARIABLE:
	{
	    s32 Index = PostData(Script,State);
	    if(Index >= State->NumberOfLocalVariables)
	    {
		LogError("%s Trying to set local variable %d to %d, but only %d exists!",Script->FunctionNames[State->ScriptNumber], Index, PopStack(State),  State->NumberOfLocalVariables);
	    }
	    else
	    {
		State->LocalVariables[Index] = PopStack(State);
	    }
	}
	break;
	case COB_POP_STATIC_VARIABLE:
	{
	    s32 Index = PostData(Script,State);
	    if(Index >= State->NumberOfStaticVariables)
	    {
		LogError("%s Trying to set static variable %d to %d, but only %d exists!", Script->FunctionNames[State->ScriptNumber],Index, PopStack(State), State->NumberOfStaticVariables);
	    }
	    else
	    {
		State->StaticVariables[Index] = PopStack(State);
	    }
	}
	    
	break;
	case COB_ADD:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 + Val2);
	}
	break;
	case COB_SUBTRACT:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 - Val2);
	}
	break;
	case COB_MULTIPLY:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 * Val2);
	}
	break;
	case COB_DIVIDE:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 / Val2);
	}
	break;
	case COB_BITWISE_AND:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val2 & Val1);
	}
	break;
	case COB_BITWISE_OR:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val2 | Val1);
	}
	break;
	case COB_BITWISE_XOR:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val2 ^ Val1 );
	}
	break;
	case COB_BITWISE_NOT:
	    PushStack(State, ~PopStack(State));
	    break;
	case COB_RANDOM:
	{
	    //TODO(Christof): Better random numbers
	    s32 LowerBound = PopStack(State);
	    s32 UpperBound = PopStack(State);
	    s32 Result = (UpperBound - LowerBound)/2;
	    Result = (rand()%(UpperBound-LowerBound))+LowerBound;
	    PushStack(State, Result);
	}
	break;
	case COB_GET_UNIT_VALUE:
	{
	    s32 GetValue = PopStack(State);
	    s32 Value = 1;
	    switch(GetValue)
	    {
	    case UNIT_VAR_BUILD_PERCENT_LEFT:
		Value = 0;
		break;
	    case UNIT_VAR_HEALTH:
		Value = 100;
		break;
	    default:
		LogWarning("Putting bogus unit value %d (%s) on the stack",GetValue, UnitVariableNames[GetValue]);
		break;
	    }
	    
	    PushStack(State, Value);
	}
	break;
	case COB_GET_VALUE:
	{
	    s32 GetValueArg = PopStack(State);
	    s32 GetValue = PopStack(State);
	    s32 Value = 1;
	    switch(GetValue)
	    {
	    case 0:
		Value=1;
		break;
	    }
	    LogWarning("Putting bogus value %d (%s) on the stack",GetValue, UnitVariableNames[GetValue]);
	    PushStack(State, Value);   
	}
	break;
	case COB_UNKNOWN5:
	case COB_UNKNOWN6:
	    //NOTE(Christof): these apparently just push 0 onto the stack?
	    LogDebug("Pushing 0 onto the stack for unknown5/6");
	    PushStack(State, 0);
	    break;
	    //TODO(Christof): investigate (TA Spring probably a good source) for the order of operands?
	    //Cob reference might work too (probably just implemented without checking these)
	case COB_LESS_THAN:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 < Val2);
	}
	break;
	case COB_LESS_THAN_OR_EQUAL:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 <= Val2);
	}
	break;
	case COB_GREATER_THAN:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 > Val2);
	}
	break;
	case COB_GREATER_THAN_OR_EQUAL:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 > Val2);
	}
	break;
	case COB_EQUAL:
	{	
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 == Val2);
	}
	break;
	case COB_NOT_EQUAL:
	{
	    s32 Val2 = PopStack(State);
	    s32 Val1 = PopStack(State);
	    PushStack(State, Val1 != Val2);
	}
	break;
	case COB_AND:
	{
	    s32 Val1 = PopStack(State);
	    s32 Val2 = PopStack(State);
	    PushStack(State, Val1 && Val2);
	}
	break;
	case COB_OR:
	{
	    s32 Val1 = PopStack(State);
	    s32 Val2 = PopStack(State);
	    PushStack(State, Val1 || Val2);
	}
	break;
	case COB_XOR:
	{
	    s32 Val1 = PopStack(State);
	    s32 Val2 = PopStack(State);
	    PushStack(State, (Val2 || Val1) && (!(Val2 && Val1)));
	}
	break;
	case COB_NOT:
	    PushStack(State,!PopStack(State));
	    break;
	case COB_START_SCRIPT:
	    CreateNewScriptState(Script,State,Pool);
	    break;
	case COB_CALL_SCRIPT2:
	    LogDebug("Call Script NUMBER 2!!!!");
	case COB_CALL_SCRIPT:
	{
	    ScriptState * NewState = CreateNewScriptState(Script,State,Pool);
	    NewState->ReturnTo = State;
	    NewState->StackData = State->StackData;
	    NewState->StackSize = State->StackSize;
	    State->BlockedOn = BLOCK_SCRIPT;
	    return -1;
	}
	break;
	case COB_JUMP:
	    //NOTE(Christof): This is unconditional, does NOT pull from the stack
	    State->ProgramCounter = PostData(Script,State);
	    break;
	case COB_RETURN:
	{
	    State->BlockedOn=BLOCK_DONE;
	    s32 Result = PopStack(State);
	    if(State->ReturnTo)
	    {
		State->ReturnTo->StackSize = State->StackSize;
		State->ReturnTo->BlockedOn = BLOCK_NOT_BLOCKED;
	    }
	    return Result;
	}
	case COB_JUMP_IF_FALSE:
	{
	    s32 NewProgramCounter = PostData(Script,State);
	    if(PopStack(State) == 0 )
	    {
		State->ProgramCounter = NewProgramCounter;
	    }
	    break;
	}
	case COB_SIGNAL:
	{
	    s32 Signal = PopStack(State);
	    for(s32 i=0;i<Pool->NumberOfScripts;i++)
	    {
		if(Pool->Scripts[i].SignalMask & Signal)
		{
		    Pool->Scripts[i].BlockedOn = BLOCK_DONE;
		}
	    }
	    if(State->BlockedOn == BLOCK_DONE)
		return -1;
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
	    s32 ExplodeType = PopStack(State);
	    s32 Piece = PostData(Script,State);
	    LogWarning("Ignoring Explosion %d directive for %s",ExplodeType,Script->PieceNames[Piece]);
	}
	break;
	case COB_SET_UNIT_VALUE:
	{
	    s32 Value = PopStack(State);
	    s32 UnitVariableNumber = PopStack(State);
	    LogWarning("Not setting Value %d (%s) to %d", UnitVariableNumber, UnitVariableNames[UnitVariableNumber], Value);
	}
	break;
	case COB_ATTACH_UNIT:
	{
	    s32 UnitID = PopStack(State);
	    s32 Piece = PopStack(State);
	    s32 Unknown = PopStack(State);
	    LogWarning("Not attaching Unit %d to %s (Unknown: %d)",UnitID,Script->PieceNames[Piece], Unknown);
	}
	break;
	case COB_DROP_UNIT:
	{
	    s32 UnitID = PopStack(State);
	    LogWarning("Not Dropping UnitID %d", UnitID);
	}
	break;
	}
    }
}


const char * Axes[]= {"X Axis","Y Axis","Z Axis"};
#define PushStack(a,b) ASASAKLSJD
#define PopStack (a) ASD:LAKSD:
#define PostData(a,b) AS:DLASKD

s32 GetStack(ScriptState * State, s32 Offset)
{
    return State->StackData[State->StackSize - Offset];
}

s32 GetNextData( UnitScript * Script,ScriptState * State, int Offset)
{
    return Script->ScriptData[State->ProgramCounter + Offset];
}

s32 OutputInstructionString(UnitScript * Script, ScriptState * State, s32 X, s32 Y, Color TextColor, s32 Offset, TextureContainer * Font, Texture2DShaderDetails * ShaderDetails)
{
    if(State->ProgramCounter + Offset > Script->ScriptDataSize)
	return -1;
    if(State->BlockedOn == BLOCK_INIT)
	return -1;
    const s32 MAX_STRING_LEN = 128;
    char InstructionString[MAX_STRING_LEN] = "UNDER CONSTRUCTION";
   
       s32 Result =  1;
    switch(Script->ScriptData[State->ProgramCounter + Offset])
    {
    case COB_MOVE:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];
	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    s32 Speed = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "Move %s along %s by %d (at %d)",PieceName, Axes[Axis], Target, Speed);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Move %s along %s by ? (at ?)",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_TURN:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];
	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    s32 Speed = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "Turn %s about %s by %d (at %d)",PieceName, Axes[Axis], Target, Speed);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Turn %s about %s by ? (at ?)",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_SPIN:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];

	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    s32 Speed = GetStack(State, 1);
	    if(Speed == 0)
	    {
		snprintf(InstructionString, MAX_STRING_LEN, "Spin %s about %s by %d",PieceName, Axes[Axis], Target);
	    }
	    {
		snprintf(InstructionString, MAX_STRING_LEN, "Spin %s about %s by %d (at %d)",PieceName, Axes[Axis], Target, Speed);
	    }
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Spin %s about %s by ? (at ?)",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_SPIN_STOP:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];

	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    s32 Speed = GetStack(State, 0);
	    if(Target == 0)
	    {
		snprintf(InstructionString, MAX_STRING_LEN, "Slow spin of %s about %s at %d",PieceName, Axes[Axis], Speed);
	    }
	    {
		snprintf(InstructionString, MAX_STRING_LEN, "Slow spin of %s about %s to %d (at %d)",PieceName, Axes[Axis], Target, Speed);
	    }
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Spin %s about %s by ? (at ?)",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_SHOW:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Show %s",PieceName);
    }
    break;
    case COB_HIDE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Hide %s",PieceName);
    }
    break;
    case COB_CACHE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Cache %s",PieceName);
    }
    break;
    case COB_DONT_CACHE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Don't Cache %s",PieceName);
    }
    break;
    case COB_MOVE_NOW:
    {	
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];
	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Move %s along %s to %d",PieceName, Axes[Axis], Target);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Move %s along %s to ?",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_TURN_NOW:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];
	if(Offset == 0 )
	{
	    s32 Target = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Turn %s about %s to %d",PieceName, Axes[Axis], Target);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Turn %s about %s to ?",PieceName, Axes[Axis]);
	}
    }
    break;
    case COB_SHADE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Shade %s",PieceName);
    }
    break;
    case COB_DONT_SHADE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	snprintf(InstructionString, MAX_STRING_LEN, "Don't Shade %s",PieceName);
    }
    break;
    case COB_EMIT_SPECIAL_EFFECTS:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];
	if(Offset == 0 )
	{
	    s32 Type = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Emit SFX %d at %s",Type, PieceName);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Emit SFX ? at %s",PieceName);
	}
    }
    break;
    case COB_WAIT_FOR_TURN:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];

	snprintf(InstructionString, MAX_STRING_LEN, "Wait for %s to turn about %s",PieceName, Axes[Axis]);
    }
	break;
    case COB_WAIT_FOR_MOVE:
    {
	s32 PieceNumber = GetNextData(Script,State, Offset + (Result++));
	s32 Axis = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[PieceNumber];

	snprintf(InstructionString, MAX_STRING_LEN, "Wait for %s to move along %s",PieceName, Axes[Axis]);
    }
	break;
    case COB_SLEEP:
	if(Offset == 0 )
	{
	    s32 Time = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Sleep %d milliseconds",Time);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Sleep ?");
	}
	  break;
    case COB_CREATE_LOCAL_VARIABLE:
	snprintf(InstructionString, MAX_STRING_LEN, "Create Local Variable");
	break;
    case COB_STACK_FLUSH:
	snprintf(InstructionString, MAX_STRING_LEN,  "Flush Stack");
	break;
    case COB_PUSH_CONSTANT:
	snprintf(InstructionString, MAX_STRING_LEN, "Push %d",GetNextData(Script,State, Offset + (Result++)));
	break;
    case COB_PUSH_LOCAL_VARIABLE:
    {
	s32 Index = GetNextData(Script,State, Offset + (Result++));
	snprintf(InstructionString, MAX_STRING_LEN, "Push Local Variable %d",Index);
    }
    break;
    case COB_PUSH_STATIC_VARIABLE:
    {
	s32 Index = GetNextData(Script,State, Offset + (Result++));
	snprintf(InstructionString, MAX_STRING_LEN, "Push Static Variable %d",Index);
    }
    break;
    case COB_POP_LOCAL_VARIABLE:
    {
	s32 Index = GetNextData(Script,State, Offset + (Result++));
	snprintf(InstructionString, MAX_STRING_LEN, "Pop Local Variable %d",Index);
    }
    break;
    case COB_POP_STATIC_VARIABLE:
    {
		s32 Index = GetNextData(Script,State, Offset + (Result++));
	snprintf(InstructionString, MAX_STRING_LEN, "Pop Static Variable %d",Index);
    }
    break;
    case COB_ADD:
	if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d + %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? + ?");
	}
	break;
    case COB_SUBTRACT:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d - %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? - ?");
	}
	
	break;
    case COB_MULTIPLY:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d * %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? * ?");
	}

	break;
    case COB_DIVIDE:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d / %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? / ?");
	}
	break;
    case COB_BITWISE_AND:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d & %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? & ?");
	}
	break;
    case COB_BITWISE_OR:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d | %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? | ?");
	}
	break;
    case COB_BITWISE_XOR:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d ^ %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? ^ ?");
	}
	break;
    case COB_BITWISE_NOT:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "~ %d",Op1);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "~ ?");
	}
	break;
    case COB_RANDOM:
    {
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "Random number between %d and %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Random Number between ? and ?");
	}
    }
    break;
    case COB_GET_UNIT_VALUE:
    {
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Get unit Value %d (%s)",Op1,UnitVariableNames[Op1]);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Get unit value ?");
	}
    }
    break;
    case COB_GET_VALUE:
    {
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Get Value %d (%s)",Op1,UnitVariableNames[Op1]);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Get Value ?");
	}
    }
    break;
    case COB_UNKNOWN5:
	snprintf(InstructionString, MAX_STRING_LEN, "UNKNOWN5");
	break;
    case COB_UNKNOWN6:
	snprintf(InstructionString, MAX_STRING_LEN, "UNKNOWN6");
	break;
    case COB_LESS_THAN:
	if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d < %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? < ?");
	}
	break;
    case COB_LESS_THAN_OR_EQUAL:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d <= %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? <= ?");
	}
	break;
    case COB_GREATER_THAN:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d > %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? > ?");
	}
	break;
    case COB_GREATER_THAN_OR_EQUAL:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d >= %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? >= ?");
	}
	break;
    case COB_EQUAL:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d == %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? == ?");
	}
	break;
    case COB_NOT_EQUAL:
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d != %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? != ?");
	}
	break;
    case COB_AND:
    
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d && %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? && ?");
	}
    
    break;
    case COB_OR:
    
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d || %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? || ?");
	}
    
    break;
    case COB_XOR:
    
		if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    s32 Op2 = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "%d ^^ %d",Op1, Op2);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "? ^^ ?");
	}
	break;

    case COB_NOT:
	if(Offset == 0 )
	{
	    s32 Op1 = GetStack(State, 0);
	    
	    snprintf(InstructionString, MAX_STRING_LEN, "! %d",Op1);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "! ?");
	}
	break;
    case COB_START_SCRIPT:
    {
	 s32 FunctionNumber = GetNextData(Script,State, Offset + (Result++));
	 s32 NumberOfArguments = GetNextData(Script,State, Offset + (Result++));
	 char * FunctionName = Script->FunctionNames[FunctionNumber];
	 //TODO(Christof): display arguments from stack if we can
	 snprintf(InstructionString, MAX_STRING_LEN, "Start %s with %d arguments", FunctionName, NumberOfArguments);
    }
	break;
    case COB_CALL_SCRIPT2:
    case COB_CALL_SCRIPT:
    {
	s32 FunctionNumber = GetNextData(Script,State, Offset + (Result++));
	s32 NumberOfArguments = GetNextData(Script,State, Offset + (Result++));
	char * FunctionName = Script->FunctionNames[FunctionNumber];
	//TODO(Christof): display arguments from stack if we can
	snprintf(InstructionString, MAX_STRING_LEN, "Call %s with %d arguments", FunctionName, NumberOfArguments);
    }
    break;
    case COB_JUMP:
    {
	//NOTE(Christof): This is unconditional, does NOT pull from the stack
	s32 JumpTo = GetNextData(Script,State, Offset + (Result++));
	snprintf(InstructionString, MAX_STRING_LEN, "Jump to %d", JumpTo);
	Result = JumpTo - (State->ProgramCounter + Offset);
    }
	break;
    case COB_RETURN:
    {
	snprintf(InstructionString, MAX_STRING_LEN, "Return");
	Result = -Offset -1;
    }
    break;
    case COB_JUMP_IF_FALSE:
    {
	s32 JumpTo = GetNextData(Script,State, Offset + (Result++));
	if(Offset == 0)
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Jump to %d if %d false", JumpTo, GetStack(State, 0));
	}
	else
	{
	snprintf(InstructionString, MAX_STRING_LEN, "Jump to %d if ? false", JumpTo);
	}
	break;
    }
    case COB_SIGNAL:
    {
	if(Offset == 0)
	{
	    s32 Signal = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Signal %d", Signal);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Signal ?");
	}
    }
    break;
    case COB_SET_SIGNAL_MASK:
    {
	if(Offset == 0)
	{
	    s32 Signal = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Set signal mask %d", Signal);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Set signal mask ?");
	}
    }
    break;
    case COB_EXPLODE:
    {
	s32 Piece = GetNextData(Script,State, Offset + (Result++));
	char * PieceName = Script->PieceNames[Piece];
	if(Offset == 0)
	{
	    s32 Type = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Explode %s of type %d", PieceName, Type);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Explode %s of type ?", PieceName);
	}
    }
    break;
    case COB_SET_UNIT_VALUE:
    {
		if(Offset == 0)
	{
	    s32 Value = GetStack(State, 0);
	    s32 UnitVarNumber = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "Set unit value %d to %d", UnitVarNumber, Value);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Set unit value ? to ?");
	}
    }
    break;
    case COB_ATTACH_UNIT:
    {
	
	if(Offset == 0)
	{
	    s32 UnitID = GetStack(State, 0);
	    s32 Piece = GetStack(State, 1);
	    s32 Unknown = GetStack(State, 1);
	    snprintf(InstructionString, MAX_STRING_LEN, "Attach UnitID %d to %s (unknown : %d)", UnitID,  Script->PieceNames[Piece], Unknown );
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Attach UnitID ? to ? (unknown : ?)");
	}
    }
    break;
    case COB_DROP_UNIT:
    {
		if(Offset == 0)
	{
	    s32 UnitID = GetStack(State, 0);
	    snprintf(InstructionString, MAX_STRING_LEN, "Drop UnitID %d", UnitID);
	}
	else
	{
	    snprintf(InstructionString, MAX_STRING_LEN, "Drop UnitID ?");
	}
    }
    break;
    default:
	Assert(!"ERRR, what?");
	break;
    }
    char OutputString[MAX_STRING_LEN];
    snprintf(OutputString, MAX_STRING_LEN, "%d) %s", State->ProgramCounter + Offset, InstructionString);
    DrawTextureFontText(OutputString, X, Y, Font , ShaderDetails, 1.0f, TextColor);
    return Result;
}
