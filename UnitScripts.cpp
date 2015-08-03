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
    if(State->StackSize >= UNIT_SCRIPT_MAX_STACK_SIZE)
    {
	LogError("%d items on the stack",State->StackSize);
	return;
    }
    State->Stack[State->StackSize++] = Value;
}

inline s32 PopStack(ScriptState * State)
{
    if(State->StackSize <= 0)
    {
	LogError("No more items on the stack");
	return -1;
    }
    return State->Stack[--State->StackSize];
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



void CreateNewScriptState(UnitScript * Script, ScriptState * State, ScriptState * NewState)
{
    s32 FunctionNumber = PostData(Script,State);
    s32 NumberOfArguments = PostData(Script,State);
    memset(NewState, 0, sizeof(ScriptState));
    for(int i=NumberOfArguments-1;i>=0;i--)
    {
	NewState->LocalVariables[i]= PopStack(State);
    }
    NewState->NumberOfLocalVariables = NumberOfArguments;
    NewState->StaticVariables = State->StaticVariables;
    NewState->NumberOfStaticVariables = State->NumberOfStaticVariables;
    NewState->ScriptNumber = FunctionNumber;
    NewState->SignalMask = State->SignalMask;
   
    NewState->TransformationDetails = State->TransformationDetails;
    NewState->NumberOfParameters = NumberOfArguments;
}

void RunScript(UnitScript * Script, ScriptState * State, Object3d * Object, ScriptStatePool * Pool)
{
    if(State->ScriptNumber <0)
	return;
    if(State->ProgramCounter > Script->ScriptDataSize)
	return;
    switch(State->BlockedOn)
    {
    case BLOCK_SCRIPT:
	//NOTE(christof): the called script will wake us on return (and kill by signal I guess?)
	return;
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
    }
    int CurrentInstructionCount=0;

    //NOTE(Christof): in cob instructions doc, top of the stack is at the BOTTOM of the list
    for(;;)
    {
	if(CurrentInstructionCount++ > MAX_INSTRUCTIONS_PER_FRAME)
	{
	    LogWarning("%s exceeded %d instructions per frame", Script->FunctionNames[State->ScriptNumber], MAX_INSTRUCTIONS_PER_FRAME);
	    return;
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
		PieceTransform->Spin[Axis]=Speed/COB_ANGULAR_FRAME_CONSTANT/6;
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
	    s32 Speed = PopStack(State);
	    s32 Acceleration = PopStack(State);
	    s32 PieceNumber = PostData(Script,State);
	    s32 Axis = PostData(Script,State);
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
		PieceTransform->SpinTarget[Axis].Acceleration = Acceleration/COB_ANGULAR_FRAME_CONSTANT;
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
	    PieceTransform->Flags &= ~OBJECT3D_FLAG_DONT_CACHE;
	}
	break;
	case COB_DONT_CACHE:
	{
	    s32 Piece = PostData(Script,State);
	    char * PieceName = Script->PieceNames[Piece];
	    Object3dTransformationDetails * PieceTransform = State->TransformationDetails;
	    PieceTransform = FindTransformationForPiece(Object, PieceTransform, PieceName);
	    PieceTransform->Flags |= OBJECT3D_FLAG_DONT_CACHE;
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
	    return;
	    break;
	case COB_WAIT_FOR_MOVE:
	    State->BlockedOn = BLOCK_MOVE;
	    State->BlockedOnPiece = PostData(Script,State);
	    State->BlockedOnAxis = PostData(Script,State);
	    return;
	    break;
	case COB_SLEEP:
	    State->BlockedOn = BLOCK_SLEEP;
	    State->BlockTime = PopStack(State);
	    return;
	case COB_CREATE_LOCAL_VARIABLE:
	    if(State->NumberOfParameters)
		State->NumberOfParameters--;
	    else
		State->NumberOfLocalVariables++;
	    break;
	case COB_STACK_FLUSH:
	    LogDebug("Flushing stack in %s",Script->FunctionNames[State->ScriptNumber]);
	    State->StackSize=0;
	    break;
	case COB_PUSH_CONSTANT:
	    PushStack(State, PostData(Script,State));
	    break;
	    //TODO(Christof): bounds checks on local/static vars
	case COB_PUSH_LOCAL_VARIABLE:
	{
	    s32 Index = PostData(Script,State);
	    if(Index>=State->NumberOfLocalVariables)
	    {
		LogError("%s Trying to push the value of local variable %d, but only %d exist, pushoing 0 instead",Script->FunctionNames[State->ScriptNumber], Index, State->NumberOfLocalVariables);
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
	    	LogError("%s Trying to push the value of static variable %d, but only %d exist, pushoing 0 instead",Script->FunctionNames[State->ScriptNumber], Index, State->NumberOfStaticVariables);
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
		LogError("%s Trying to set local variable %d to %d, but only %d exists!", Script->FunctionNames[State->ScriptNumber],Index, PopStack(State), State->NumberOfStaticVariables);
	    }
	    else
	    {
		State->StaticVariables[Index] = PopStack(State);
	    }
	}
	    
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
	    case 17:
		//17 is buildpercent - this will make all the wait for completiong stuff work I hope - 0 is fully built?
		Value=0;
		break;
	    case 18:
		Value=0;
		break;
	    case 4:
		//health - seems to be in %
		Value=0;
		break;
	    default:
		LogWarning("Putting bogus unit value %d on the stack",GetValue);
		break;
	    }
	    
	    PushStack(State, Value);
	}
	break;
	case COB_GET_VALUE:
	{
	    s32 GetValue = PopStack(State);
	    s32 Value = 1;
	    switch(GetValue)
	    {
	    case 0:
		Value=1;
		break;
	    }
	    LogWarning("Putting bogus value %d on the stack",GetValue);
	    PushStack(State, Value);   
	}
	break;
	case COB_UNKNOWN5:
	case COB_UNKNOWN6:
	    //NOTE(Christof): these apparently just push 0 onto the stack?
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
	{
	    int ScriptIndex = Pool->NumberOfScripts++;

	    if(ScriptIndex >= SCRIPT_POOL_SIZE)
	    {
		Pool->NumberOfScripts--;
		s32  FunctionNumber = PostData(Script,State);
		PostData(Script,State);

		LogError("Too many scripts, failed to start new script %s", Script->FunctionNames[FunctionNumber]);
	    }
	    else
	    {
		ScriptState * NewState = &Pool->Scripts[ScriptIndex];
		CreateNewScriptState(Script,State,NewState);
		if(NewState->ScriptNumber >= Script->NumberOfFunctions)
		    LogDebug("%d over %d", NewState->ScriptNumber, Script->NumberOfFunctions);
	    }
	}
	break;
	case COB_CALL_SCRIPT2:
	    LogDebug("Call Script NUMBER 2!!!!");
	case COB_CALL_SCRIPT:
	{
	    int ScriptIndex = Pool->NumberOfScripts++;
	    if(ScriptIndex >= SCRIPT_POOL_SIZE)
	    {
		Pool->NumberOfScripts--;
		s32  FunctionNumber = PostData(Script,State);
		PostData(Script,State);

		LogError("Too many scripts, failed to call new script %s", Script->FunctionNames[FunctionNumber]);
	    }
	    else
	    {
		ScriptState * NewState = &Pool->Scripts[ScriptIndex];
		CreateNewScriptState(Script,State,NewState);
		NewState->ReturnTo = State;
		State->BlockedOn = BLOCK_SCRIPT;

		if(NewState->ScriptNumber >= Script->NumberOfFunctions)
		    LogDebug("%d over %d", NewState->ScriptNumber, Script->NumberOfFunctions);
		return;
	    }
	}
	break;
	case COB_JUMP:
	    //NOTE(Christof): This is unconditional, does NOT pull from the stack
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
	    LogError("We do not currently handle signals %d",Signal);
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
	    LogWarning("Not setting Value %d to %d", UnitVariableNumber, Value);
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
