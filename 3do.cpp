


struct FILE_Object3dHeader
{
    int32_t Version;
    int32_t NumberOfVertexes;
    int32_t NumberOfPrimitives;
    int32_t OffsetToSelectionPrimitive;//Appears to be index into primitive array
    int32_t XFromParent;
    int32_t YFromParent;
    int32_t ZFromParent;
    int32_t OffsetToObjectName;
    int32_t Always_0;
    int32_t OffsetToVertexArray;
    int32_t OffsetToPrimitiveArray;
    int32_t OffsetToSiblingObject;
    int32_t OffsetToChildObject;
};

struct FILE_Vertex
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct FILE_Primitive
{
    int32_t ColorIndex;
    int32_t NumberOfVertexIndexes;
    int32_t Always_0;
    int32_t OffsetToVertexIndexArray;
    int32_t OffsetToTextureName;
    //Apparently Cavedog editor specific:
    int32_t Unknown_1;
    int32_t Unknown_2;
    int32_t Unknown_3;    
};


const double TA_TO_GL_SCALE=1.0f/2500000.0f;

struct Position3d
{
    double X,Y,Z;
};

struct Object3d
{
    char * Name;
    Object3d * Children;
    int NumberOfChildren;
    Position3d Position;
};


//TODO(Christof): Figure out if we can/need to use bindless textures or something similar for rendering, for now lets just go with the simple solution
//NOTE: Palette for non-textured primites is in "palettes/PALETTE.PAL"

int Count3DOChildren(char * Buffer,FILE_Object3dHeader * header)
{
    int NumChildren = 0;
    if(header->OffsetToChildObject)
    {
	NumChildren=1;
	header = (FILE_Object3dHeader *)(Buffer+header->OffsetToChildObject);
	while(header->OffsetToSiblingObject)
	{
	    header = (FILE_Object3dHeader *)(Buffer+header->OffsetToSiblingObject);
	    NumChildren++;
	}
    }
    return NumChildren;
}

bool32 Load3DOFromBuffer(char * Buffer, Object3d * Object, int Offset=0)
{
    FILE_Object3dHeader * header = (FILE_Object3dHeader *)(Buffer+Offset);
    if(header->Version != 1)
    {
	LogError("Unknown Version in 3DO file: %d",header->Version);
	return 0;
    }
    if(header->OffsetToSiblingObject && !Offset)
    {
	LogError("Top Level 3DO Objects should not have siblings!");
	return 0;
    }

    int NameLength=strlen(Buffer + header->OffsetToObjectName)+1;
    Object->Name = (char *)malloc(NameLength);
    memcpy(Object->Name,Buffer+header->OffsetToObjectName,NameLength);
    LogDebug(Object->Name);
    Object->Position.X=header->XFromParent*TA_TO_GL_SCALE;
    Object->Position.Y=header->YFromParent*TA_TO_GL_SCALE;
    Object->Position.Z=header->ZFromParent*TA_TO_GL_SCALE;
    
    

    

    Object->NumberOfChildren = Count3DOChildren(Buffer,header);
    Object->Children = (Object3d *)malloc(sizeof(Object3d)*Object->NumberOfChildren);
    int ChildOffset=header->OffsetToChildObject;
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Load3DOFromBuffer(Buffer, &(Object->Children[i]),ChildOffset);
	header = (FILE_Object3dHeader *)(Buffer+ChildOffset);
	ChildOffset=header->OffsetToSiblingObject;
    }

    return 1;
}

/**
   Frees all contents allocated by loading the object, leaving it in a state to be reused for loading without memory leaks
 **/
void Unload3DO(Object3d * Object)
{
    if(!Object)
	return;
    if(Object->Name)
	free(Object->Name);
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Unload3DO(&Object->Children[i]);
    }
    if(Object->Children)
	free(Object->Children);
}
