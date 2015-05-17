

#pragma pack(push,1)
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

struct FILE_Object3dVertex
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct FILE_Object3dPrimitive
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
#pragma pack(pop)


struct Position3d
{
    float X,Y,Z;
};

#define MAX_3DO_NAME_LENGTH 32
struct Object3d
{
    char Name[MAX_3DO_NAME_LENGTH];
    Object3d * Children;
    int NumberOfChildren;
    Position3d Position;
    int NumberOfPrimitives;
    struct Object3dPrimitive * Primitives;
    int NumberOfVertices;
    GLfloat * Vertices;
    GLuint VertexBuffer;
    int NumTriangles;
};

struct Object3dPrimitive
{
    Texture * Texture;
    int NumberOfVertices;
    int ColorIndex;
    int * VertexIndexes;
};

void FillObject3dData(GLfloat* Data, int CurrentTriangle,int * VertexIndices,GLfloat * UV, Texture * Texture, Object3d * Object, Object3dPrimitive * Primitive,uint8_t * PaletteData)
{
    int Offset=CurrentTriangle*(3+2+3)*3;
    Data+=Offset;

    for(int i=0;i<3;i++)
    {
	Data[i*8+0]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+0];
	Data[i*8+1]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+1];
	Data[i*8+2]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+2];

	Data[i*8+3]=Texture->U+Texture->Width*UV[i*2+0];
	Data[i*8+4]=Texture->V+Texture->Height*UV[i*2+1];
	

	Data[i*8+5]=(uint8_t)PaletteData[Primitive->ColorIndex*4+0]/255.0f;
	Data[i*8+6]=(uint8_t)PaletteData[Primitive->ColorIndex*4+1]/255.0f;
	Data[i*8+7]=(uint8_t)PaletteData[Primitive->ColorIndex*4+2]/255.0f;

    }
}

Texture NoTexture={"",0,-2,-2,-1,-1,0};

bool32 PrepareObject3dForRendering(Object3d * Object,uint8_t * PaletteData)
{
    if(Object->VertexBuffer)
    {
	glDeleteVertexArrays(1,&Object->VertexBuffer);
    }

    Object->NumTriangles=0;
    for(int i=0;i<Object->NumberOfPrimitives;i++)
    {
	Object->NumTriangles += Object->Primitives[i].NumberOfVertices-2>0?Object->Primitives[i].NumberOfVertices-2:0;
    }
    //GLfloat Data[Object->NumTriangles * (3+2+3)*3];//3 coord, 2 tex, 3 color
    STACK_ARRAY(Data,Object->NumTriangles * (3+2+3)*3, GLfloat);
    //NOTE: signal no texture in some way, perhaps negative texture coords?
    int CurrentTriangle=0;
    for(int PrimitiveIndex=0;PrimitiveIndex<Object->NumberOfPrimitives;PrimitiveIndex++)
    {
	Object3dPrimitive * CurrentPrimitive=&Object->Primitives[PrimitiveIndex];
	Texture * Texture=CurrentPrimitive->Texture;
	if(!Texture)
	    Texture=&NoTexture;
	switch(CurrentPrimitive->NumberOfVertices)
	{
	case 1:
	    LogDebug("ignoring point in %s",Object->Name);
	    break;
	case 2:
	    LogDebug("ignoring line in %s",Object->Name);
	    break;
	case 3:
	{
	    GLfloat UVCoords[]={0,1, 1,1, 0,0};
	    int Vertexes[]={0,1,2};
	    FillObject3dData(Data,CurrentTriangle,Vertexes,UVCoords,Texture,Object,CurrentPrimitive,PaletteData);
	    CurrentTriangle++;
	}
	    break;
	case 4:
	{
	    GLfloat UVCoords1[]={1,0, 0,1, 1,1};
	    int Vertexes1[]={0,2,3};
	    FillObject3dData(Data,CurrentTriangle,Vertexes1,UVCoords1,Texture,Object,CurrentPrimitive,PaletteData);
	    CurrentTriangle++;
	    GLfloat UVCoords2[]={1,0, 0,0, 0,1};
	    int Vertexes2[]={0,1,2};
	    FillObject3dData(Data,CurrentTriangle,Vertexes2,UVCoords2,Texture,Object,CurrentPrimitive,PaletteData);
	    CurrentTriangle++;
	}
	    break;
	default:
	{
	    GLfloat UVCoords[6];
	    //Map sin/cos unit circle at (0,0) onto 1/2 unit circle at (0.5,0.5)
	    UVCoords[2*2]=0.5f*(float)(1.0f-(sin((2.0f*(CurrentPrimitive->NumberOfVertices-1)+1)*PI/float(CurrentPrimitive->NumberOfVertices))/cos(PI/CurrentPrimitive->NumberOfVertices)));
	    UVCoords[2*2+1]=0.5f*(float)(1-(cos(PI/CurrentPrimitive->NumberOfVertices*(2.0f*(CurrentPrimitive->NumberOfVertices-1)+1))/cos(PI/CurrentPrimitive->NumberOfVertices)));

	    for(int i=0;i<CurrentPrimitive->NumberOfVertices-2;i++)
	    {
		int Vertexes[]={i,i+1,CurrentPrimitive->NumberOfVertices-1};
			
		for(int j=0;j<2;j++)
		{
		    //Map sin/cos unit circle at (0,0) onto 1/2 unit circle at (0.5,0.5)
		    UVCoords[j*2]=0.5f*(float)(1-(sin((2.0f*(i+j)+1)*PI/float(CurrentPrimitive->NumberOfVertices))/cos(PI/CurrentPrimitive->NumberOfVertices)));
		    UVCoords[j*2+1]=0.5f*(float)(1-(cos(PI/CurrentPrimitive->NumberOfVertices*(2.0f*(i+j)+1))/cos(PI/CurrentPrimitive->NumberOfVertices)));
		}
		FillObject3dData(Data,CurrentTriangle,Vertexes,UVCoords,Texture,Object,CurrentPrimitive,PaletteData);
		CurrentTriangle++;
	    }
	}
	    break;
	}
    }
    
    glGenVertexArrays(1,&Object->VertexBuffer);
    glBindVertexArray(Object->VertexBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*Object->NumTriangles*(3+2+3)*3,Data,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+2+3),(GLvoid*)(sizeof(GLfloat)*5));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    //glDeleteBuffers(1,&VertexBuffer);

    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	if(!PrepareObject3dForRendering(&Object->Children[i],PaletteData))
	    return 0;
    }
	      
    
    return 1;
}

struct Object3dTransformationDetails
{
    
};


void RenderObject3d(Object3d * Object,Object3dTransformationDetails * TransformationDetails,GLuint ModelMatrixLocation, Matrix ParentMatrix=Matrix())
{
    //TODO(Christof): Actually make use of TransformationDetails
    Matrix CurrentMatrix;
    CurrentMatrix.SetTranslation(Object->Position.X,Object->Position.Y,Object->Position.Z);
    CurrentMatrix = CurrentMatrix * ParentMatrix;

    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	RenderObject3d(&Object->Children[i],TransformationDetails,ModelMatrixLocation,CurrentMatrix);
    }
    CurrentMatrix.Upload(ModelMatrixLocation);
    glBindVertexArray(Object->VertexBuffer);
    glDrawArrays(GL_TRIANGLES, 0, Object->NumTriangles*3);
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed to render : %s",gluErrorString(ErrorValue));
    }
}





//TODO(Christof): Load Primitives from file to memory
//TODO(Christof): Generate Collision Meshes from Primitives?
//TODO(Christof): Generate Render Data from Primitives


/*TODO(Christof): Figure out if we can/need to use bindless textures or something similar for rendering, for now lets just go with the simple solution
 Probably just load all the textures into one big (1024x1024 or 2048x2048 - will need to check what size we need) texture and use offsets, since the textures are all quite small*/

int Count3DOChildren(uint8_t * Buffer,FILE_Object3dHeader * header)
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




bool32 Load3DOFromBuffer(uint8_t * Buffer, Object3d * Object,int NextTexture,Texture * Textures, MemoryArena * GameArena, int Offset=0)
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

    //TODO(Christof): Bounds check
    int NameLength=(int)strlen((char*)Buffer + header->OffsetToObjectName)+1;
    memcpy(Object->Name,Buffer+header->OffsetToObjectName,NameLength);
    Object->Position.X=header->XFromParent*TA_TO_GL_SCALE;
    Object->Position.Y=header->YFromParent*TA_TO_GL_SCALE;
    Object->Position.Z=header->ZFromParent*TA_TO_GL_SCALE;

    FILE_Object3dPrimitive * Primitives=(FILE_Object3dPrimitive*)(Buffer+header->OffsetToPrimitiveArray);
    FILE_Object3dPrimitive * CurrentPrimitive = Primitives;
    Object->NumberOfPrimitives = header->NumberOfPrimitives;
    Object->Primitives = PushArray(GameArena,Object->NumberOfPrimitives,Object3dPrimitive);
    for(int i=0;i<header->NumberOfPrimitives;i++)
    {
	if(CurrentPrimitive->OffsetToTextureName)
	{
	    Object->Primitives[i].Texture = GetTexture((char*)Buffer+CurrentPrimitive->OffsetToTextureName,0,NextTexture,Textures);
	    if(!Object->Primitives[i].Texture)
	    {
		LogError("Could not get texture for primitive %d in %s, %s",i,Object->Name,Buffer+CurrentPrimitive->OffsetToTextureName);
	    }
	}
	else
	{
	    Object->Primitives[i].Texture=0;
	}
	Object->Primitives[i].NumberOfVertices = CurrentPrimitive->NumberOfVertexIndexes;
	Object->Primitives[i].VertexIndexes = PushArray(GameArena,Object->Primitives[i].NumberOfVertices,int);
	int16_t * VertexIndexes = (int16_t *)(Buffer + CurrentPrimitive->OffsetToVertexIndexArray);
	for(int j=0;j<Object->Primitives[i].NumberOfVertices;j++)
	    Object->Primitives[i].VertexIndexes[j]=VertexIndexes[j];
	
	Object->Primitives[i].ColorIndex = CurrentPrimitive->ColorIndex & 255;

	CurrentPrimitive++;
    }

    Object->NumberOfVertices = header->NumberOfVertexes;
    Object->Vertices = PushArray(GameArena,3*Object->NumberOfVertices,float);

    FILE_Object3dVertex * Vertices= (FILE_Object3dVertex *)(Buffer + header->OffsetToVertexArray);
    for(int i=0;i<Object->NumberOfVertices;i++)
    {
	Object->Vertices[i*3+0]=Vertices[i].x*TA_TO_GL_SCALE;
	Object->Vertices[i*3+1]=Vertices[i].y*TA_TO_GL_SCALE;
	Object->Vertices[i*3+2]=Vertices[i].z*TA_TO_GL_SCALE;
    }
    
    

    Object->NumberOfChildren = Count3DOChildren(Buffer,header);
    Object->Children = PushArray(GameArena,Object->NumberOfChildren,Object3d);
    memset(Object->Children,0,sizeof(Object3d)*Object->NumberOfChildren);
    int ChildOffset=header->OffsetToChildObject;
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Load3DOFromBuffer(Buffer, &(Object->Children[i]),NextTexture,Textures,GameArena,ChildOffset);
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
    if(Object->VertexBuffer)
	glDeleteBuffers(1,&Object->VertexBuffer);
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Unload3DO(&Object->Children[i]);
    }
}
