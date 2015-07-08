 
const int32_t TranslationXAxisModModel = 1;
const int32_t TranslationYAxisModModel = 1;
const int32_t TranslationZAxisModModel = -1;

const int32_t TranslationXAxisModScript = -1;
const int32_t TranslationYAxisModScript = 1;
const int32_t TranslationZAxisModScript = 1;

const int32_t RotationXAxisMod = 1;
const int32_t RotationYAxisMod = 1;
const int32_t RotationZAxisMod = -1;


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
    int NumLines;
    GLuint LineBuffer;
    int TextureOffset;
    GLuint TextureCoordBuffer;
};

struct Object3dPrimitive
{
    Texture * Texture;
    int NumberOfVertices;
    int ColorIndex;
    int * VertexIndexes;
};

void FillObject3dData(GLfloat* Data, int CurrentTriangle,int * VertexIndices,GLfloat * UV, Texture * Texture, Object3d * Object, Object3dPrimitive * Primitive,uint8_t * PaletteData,int TextureOffset, GLfloat * TextureData)
{
    int Offset=CurrentTriangle*(3+3)*3;
    Data+=Offset;
    float U=Texture->U;
    float V=Texture->V;
    float Width = Texture->Widths[TextureOffset];
    float Height = Texture->Heights[TextureOffset];
    for(int i=0;i<TextureOffset;i++)
    {
	U+=Texture->Widths[i];
    }
    TextureData += (CurrentTriangle * 4)*3;
    for(int i=0;i<3;i++)
    {
	Data[i*6+0]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+0];
	Data[i*6+1]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+1];
	Data[i*6+2]=Object->Vertices[Primitive->VertexIndexes[VertexIndices[i]]*3+2];

	

	Data[i*6+3]=(uint8_t)PaletteData[Primitive->ColorIndex*4+0]/255.0f;
	Data[i*6+4]=(uint8_t)PaletteData[Primitive->ColorIndex*4+1]/255.0f;
	Data[i*6+5]=(uint8_t)PaletteData[Primitive->ColorIndex*4+2]/255.0f;

	TextureData[i*4+0]=(U+Width*UV[i*3+0])*UV[i*3+2];
	TextureData[i*4+1]=(V+Height*UV[i*3+1])*UV[i*3+2];
	TextureData[i*4+2]=0;
	TextureData[i*4+3]=UV[i*3+2];

    }
}

Texture NoTexture={"",1,-2,-2,{0},{0}};
bool32 TextureIsSideTexture(Texture * Texture)
{
    //NOTE(Christof): only the side textures seem to have 10 frames?
    //                makes sense, there are up to 10 players per game afaict
    return Texture->NumberOfFrames ==10 ;
}

#define TEXTURE_DEBUG 0

#if TEXTURE_DEBUG
int32_t DEBUG_done = 0;
#endif

bool32 IsAirPadTexture(Texture * Texture)
{
#if TEXTURE_DEBUG
    if(!DEBUG_done)
    {
	LogDebug("Texture name: %s",Texture->Name);
    }
#endif
    return strstr(Texture->Name, "CorSe11a")!=0 || strstr(Texture->Name, "Helipad")!=0;
}

union v3
{
    float v[3];
    struct
    {
	float x,y,z;
    };
    struct
    {
	float r,g,b;
    };
};


float dot(v3 v1, v3 v2)
{
    return v1.x*v2.x+
	v1.y*v2.y+
	v1.z*v2.z;
}

v3 cross(v3 u, v3 v)
{
    v3 Result;

    Result.x = u.y*v.z - u.z*v.y;
    Result.y = u.z*v.x - u.x*v.z;
    Result.z = u.x*v.y - u.y*v.x;
    
    return Result;
}

v3 v3_from_array(float v[3])
{
    v3 Result;
    Result.x = v[0];
    Result.y = v[1];
    Result.z = v[2];
    return Result;
}

v3 sub(v3 sub, v3 from)
{
    v3 Result;
    Result.x = from.x - sub.x;
    Result.y = from.y - sub.y;
    Result.z = from.z - sub.z;
    return Result;
}

v3 mult(v3 m, float s)
{
    v3 Result;
    Result.x = m.x * s;
    Result.y = m.y * s;
    Result.z = m.z * s;
    return Result;
}

v3 add(v3 v1, v3 v2)
{
    v3 Result;
    Result.x = v1.x + v2.x;
    Result.y = v1.y + v2.y;
    Result.z = v1.z + v2.z;
    return Result;
}

float length(v3 v)
{
    return sqrt(v.x*v.x+ v.y*v.y + v.z*v.z);
}

bool32 Object3dRenderingPrep(Object3d * Object,uint8_t * PaletteData,int32_t Side)
{
    if(!Object->NumTriangles)
    {
	Object->NumTriangles=0;
	Object->NumLines = 0;
	for(int i=0;i<Object->NumberOfPrimitives;i++)
	{
	    Object->NumTriangles += Object->Primitives[i].NumberOfVertices-2>0?Object->Primitives[i].NumberOfVertices-2:0;
	    Object->NumLines += Object->Primitives[i].NumberOfVertices == 2;
	}
    }
    //GLfloat Data[Object->NumTriangles * (3+2+3)*3];//3 coord, 2 tex, 3 color
    STACK_ARRAY(Data,Object->NumTriangles * (3+3)*3, GLfloat);
    STACK_ARRAY(LineData,Object->NumLines * (3+4+3)*2, GLfloat);
    STACK_ARRAY(TextureData,Object->NumTriangles * (4)*3, GLfloat);
    //NOTE: signal no texture in some way, perhaps negative texture coords?
    int CurrentTriangle=0;
    int CurrentLine = 0;
    for(int PrimitiveIndex=0;PrimitiveIndex<Object->NumberOfPrimitives;PrimitiveIndex++)
    {
	Object3dPrimitive * CurrentPrimitive=&Object->Primitives[PrimitiveIndex];
	Texture * Texture=CurrentPrimitive->Texture;
	if(!Texture)
	    Texture=&NoTexture;
	int32_t TextureOffset = Object->TextureOffset%Texture->NumberOfFrames;
	if(TextureIsSideTexture(Texture))
	{
	    TextureOffset = Side;
	}
	switch(CurrentPrimitive->NumberOfVertices)
	{
	case 1:
	    LogDebug("ignoring point in %s",Object->Name);
	    break;
	case 2:
	    LineData[CurrentLine*2*10 + 0] = Object->Vertices[CurrentPrimitive->VertexIndexes[0]*3+0];
	    LineData[CurrentLine*2*10 + 1]= Object->Vertices[CurrentPrimitive->VertexIndexes[0]*3+1];
	    LineData[CurrentLine*2*10 + 2]= Object->Vertices[CurrentPrimitive->VertexIndexes[0]*3+2];

	    LineData[CurrentLine*2*10 + 5]= 0;
	    LineData[CurrentLine*2*10 + 6]= 1;
	    
	    LineData[CurrentLine*2*10 + 7]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+0]/255.0f;
	    LineData[CurrentLine*2*10 + 8]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+1]/255.0f;
	    LineData[CurrentLine*2*10 + 9]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+2]/255.0f;

	    
	    LineData[CurrentLine*2*10 + 10] = Object->Vertices[CurrentPrimitive->VertexIndexes[1]*3+0];
	    LineData[CurrentLine*2*10 + 11]= Object->Vertices[CurrentPrimitive->VertexIndexes[1]*3+1];
	    LineData[CurrentLine*2*10 + 12]= Object->Vertices[CurrentPrimitive->VertexIndexes[1]*3+2];

	    LineData[CurrentLine*2*10 + 13]= Texture->U+Texture->Widths[0];
	    LineData[CurrentLine*2*10 + 14]= Texture->V+Texture->Heights[0];
	    LineData[CurrentLine*2*10 + 15]= 0;
	    LineData[CurrentLine*2*10 + 16]= 1;
	    
	    
	    LineData[CurrentLine*2*10 + 17]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+0]/255.0f;
	    LineData[CurrentLine*2*10 + 18]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+1]/255.0f;
	    LineData[CurrentLine*2*10 + 19]= (uint8_t)PaletteData[CurrentPrimitive->ColorIndex*4+2]/255.0f;
	    CurrentLine++;
	    break;
	case 3:
	{
	    GLfloat UVCoords[]={0,1, 0,0, 1,1};
	    int Vertexes[]={0,2,1};
	    FillObject3dData(Data,CurrentTriangle,Vertexes,UVCoords,Texture,Object,CurrentPrimitive,PaletteData,TextureOffset,TextureData);
	    CurrentTriangle++;
	}
	    break;
	case 4:
	{
	    //GLfloat UVCoords1[]={0,0, 1,0,  1,1 };
	    int Vertexes[][3]={{0,3,2},{0,2,1}};
//	    Object->Vertices[Primitive->VertexIndexes[VertexIndices
	    v3 P0 = v3_from_array(&Object->Vertices[CurrentPrimitive->VertexIndexes[Vertexes[0][0]]*3]);
	    v3 P1 = v3_from_array(&Object->Vertices[CurrentPrimitive->VertexIndexes[Vertexes[1][2]]*3]);
	    v3 P2 = v3_from_array(&Object->Vertices[CurrentPrimitive->VertexIndexes[Vertexes[0][2]]*3]);
	    v3 P3 = v3_from_array(&Object->Vertices[CurrentPrimitive->VertexIndexes[Vertexes[0][1]]*3]);

	    
	    v3 P03 = sub(P0, P3);
	    v3 P20 = sub(P2, P0);
	    v3 P31 = sub(P3, P1);
	    v3 n = cross( cross(P31,P20),P31);
	    v3 Center = add(mult(P20,dot(P03, n)/dot(P20,n)),P0);
	    float d[4] = {};
	    d[0] = length(sub(P0,Center));
	    d[1] = length(sub(P1,Center));
	    d[2] = length(sub(P2,Center));
	    d[3] = length(sub(P3,Center));
	    
	    float mod[4]= {1,1,1,1};
	    #if 1
	    mod[0] = (d[0]+d[2])/d[2];
	    mod[1] = (d[1]+d[3])/d[3];
	    mod[2] = (d[2]+d[0])/d[0];
	    mod[3] = (d[3]+d[1])/d[1];
	    #endif


	    
	    GLfloat NormalUVCoords[][9]={{1,0,mod[0],  1,1,mod[3],   0,1,mod[2] },{1,0,mod[0],  0,1,mod[2],  0,0,mod[1]}};
	    GLfloat AirPadUVCoords[][9]={{0,0,mod[0],  1,0,mod[3],   1,1,mod[2] },{0,0,mod[0],  1,1,mod[2],  0,1,mod[1]}};

	    
	    if(IsAirPadTexture(Texture))
	    {
		FillObject3dData(Data,CurrentTriangle,Vertexes[0],AirPadUVCoords[0],Texture,Object,CurrentPrimitive,PaletteData,TextureOffset,TextureData);
		CurrentTriangle++;
		//GLfloat UVCoords2[]={0,0, 1,1, 0,1};
		FillObject3dData(Data,CurrentTriangle,Vertexes[1],AirPadUVCoords[1],Texture,Object,CurrentPrimitive,PaletteData,TextureOffset,TextureData);
		CurrentTriangle++;
	    }
	    else
	    {
		FillObject3dData(Data,CurrentTriangle,Vertexes[0],NormalUVCoords[0],Texture,Object,CurrentPrimitive,PaletteData,TextureOffset, TextureData);
		CurrentTriangle++;
		//GLfloat UVCoords2[]={0,0, 1,1, 0,1};
		FillObject3dData(Data,CurrentTriangle,Vertexes[1],NormalUVCoords[1],Texture,Object,CurrentPrimitive,PaletteData,TextureOffset, TextureData);
		CurrentTriangle++;
	    }


	   
	    
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
		FillObject3dData(Data,CurrentTriangle,Vertexes,UVCoords,Texture,Object,CurrentPrimitive,PaletteData,TextureOffset,TextureData);
		CurrentTriangle++;
	    }
	}
	    break;
	}
    }

    if(!Object->VertexBuffer)
    {
    //VERTEX DATA
    glGenVertexArrays(1,&Object->VertexBuffer);
    glBindVertexArray(Object->VertexBuffer);

    GLuint VertexBuffer;
    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*Object->NumTriangles*(3+3)*3,Data,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+3),0);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+3),(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);

    //TEXTURES

    glGenBuffers(1,&Object->TextureCoordBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,Object->TextureCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*Object->NumTriangles*(4)*3,TextureData,GL_STATIC_DRAW);
    
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(4),0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //LINES
    glGenVertexArrays(1,&Object->LineBuffer);
    glBindVertexArray(Object->LineBuffer);

    glGenBuffers(1,&VertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*Object->NumLines*(3+4+3)*2,LineData,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+4+3),0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+4+3),(GLvoid*)(sizeof(GLfloat)*3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*(3+4+3),(GLvoid*)(sizeof(GLfloat)*7));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
	
    }
    else
    {
	glBindVertexArray(Object->VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER,Object->TextureCoordBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0,sizeof(GLfloat)*4*Object->NumTriangles * 3 ,TextureData);
    }



    return 1;
}

enum
{
    TA_AXIS_X,
    TA_AXIS_Y,
    TA_AXIS_Z,
    TA_AXIS_NUM
};

struct RotationDetails
{
    float Heading;
    float Speed;
};

struct MovementDetails
{
    float Destination;
    float Speed;
};

struct SpinDetails
{
    float Speed;//NOTE(Christof): Docs on this are unclear, from docs it seems like this is the initial not target speed, I recall that in game stuff spins up to a target speed though (e.g. metal extractor, radar ) - this may need some investigation.
    float Acceleration;//NOTE(Christof): Always positive, i.e. |a|
};


const uint32_t OBJECT3D_FLAG_HIDE = 1;
const uint32_t OBJECT3D_FLAG_DONT_CACHE = 2;
const uint32_t OBJECT3D_FLAG_DONT_SHADE = 4;
struct Object3dTransformationDetails
{
    RotationDetails RotationTarget[TA_AXIS_NUM];
    MovementDetails MovementTarget[TA_AXIS_NUM];
    SpinDetails SpinTarget[TA_AXIS_NUM];
    
    float Rotation[TA_AXIS_NUM];
    float Movement[TA_AXIS_NUM];
    float Spin[TA_AXIS_NUM];

    Object3dTransformationDetails * Children;
    uint32_t Flags;
};

void InitTransformationDetails(Object3d * Object, Object3dTransformationDetails *  TransformationDetails, MemoryArena * GameArena)
{
    memset(TransformationDetails, 0, sizeof(Object3dTransformationDetails));
    TransformationDetails->Children = PushArray(GameArena, Object->NumberOfChildren,Object3dTransformationDetails);
    for(int i=0;i<Object->NumberOfChildren;i++)
	InitTransformationDetails(& Object->Children[i], &TransformationDetails->Children[i], GameArena);
}

void UpdateTransformationDetails(Object3d* Object, Object3dTransformationDetails * TransformationDetails,float TimeStep)
{
    if(!Object || ! TransformationDetails)
	return;
    for(int i=0;i<TA_AXIS_NUM;i++)
    {
	if(TransformationDetails->RotationTarget[i].Speed != 0)
	{
	    float Current = TransformationDetails->Rotation[i];
	    float Target = TransformationDetails->RotationTarget[i].Heading;
	    float Speed = TransformationDetails->RotationTarget[i].Speed;
	    float MaxChange = Speed * TimeStep;
	    float Difference = Target - Current;
	    if(Difference >0)
	    {
		if(Difference <=MaxChange)
		{
		    TransformationDetails->Rotation[i] = TransformationDetails->RotationTarget[i].Heading;
		    TransformationDetails->RotationTarget[i].Speed=0;
		}
		else
		    TransformationDetails->Rotation[i] += MaxChange;
	    }
	    else if(Difference <0)
	    {
		if(Difference >= MaxChange)
		{
		    TransformationDetails->Rotation[i] = TransformationDetails->RotationTarget[i].Heading;
		    TransformationDetails->RotationTarget[i].Speed=0;
		}
		else
		    TransformationDetails->Rotation[i] -= MaxChange;
	    }
	    else
	    {
		TransformationDetails->RotationTarget[i].Speed=0;
	    }
	}
	if(TransformationDetails->MovementTarget[i].Speed != 0)
	{
	    float Speed = TransformationDetails->MovementTarget[i].Speed;
	    float MaxChange = Speed*TimeStep;
	    float Target = TransformationDetails->MovementTarget[i].Destination;
	    float Current = TransformationDetails->Movement[i];
	    float Difference = Target - Current;
	    if(Difference >0)
	    {
		if(Difference <=MaxChange)
		{
		    TransformationDetails->Movement[i] = TransformationDetails->MovementTarget[i].Destination;
		    TransformationDetails->MovementTarget[i].Speed=0;
		}
		else
		    TransformationDetails->Movement[i] += MaxChange;
	    }
	    else if(Difference <0)
	    {
		if(Difference >= MaxChange)
		{
		    TransformationDetails->Movement[i] = TransformationDetails->MovementTarget[i].Destination;
		    TransformationDetails->MovementTarget[i].Speed=0;
		}
		else
		    TransformationDetails->Movement[i] -= MaxChange;
	    }
	    else
	    {
		TransformationDetails->MovementTarget[i].Speed=0;
	    }
	}
	if(TransformationDetails->SpinTarget[i].Acceleration != 0)
	{
	    //NOTE(Christof): Docs indicate acceleration is magnitude only
	    if(TransformationDetails->SpinTarget[i].Speed > 0)
	    {
		TransformationDetails->Spin[i] += TransformationDetails->SpinTarget[i].Acceleration;
		if(TransformationDetails->Spin[i] > TransformationDetails->SpinTarget[i].Speed)
		{
		    TransformationDetails->Spin[i] = TransformationDetails->SpinTarget[i].Speed;
		    TransformationDetails->SpinTarget[i].Acceleration = 0;
		}
	    }
	    else if(TransformationDetails->SpinTarget[i].Speed < 0)
	    {
		TransformationDetails->Spin[i] -= TransformationDetails->SpinTarget[i].Acceleration;
		if(TransformationDetails->Spin[i] < TransformationDetails->SpinTarget[i].Speed)
		{
		    TransformationDetails->Spin[i] = TransformationDetails->SpinTarget[i].Speed;
		    TransformationDetails->SpinTarget[i].Acceleration = 0;
		}
	    }
	    else
	    {
		//NOTE(Christof): Spin-Stop
		if(TransformationDetails->Spin[i] > 0)
		{
		    TransformationDetails->Spin[i] -= TransformationDetails->SpinTarget[i].Acceleration;
		    if(TransformationDetails->Spin[i] > TransformationDetails->SpinTarget[i].Speed)
		    {
			TransformationDetails->Spin[i] = TransformationDetails->SpinTarget[i].Speed;
			TransformationDetails->SpinTarget[i].Acceleration = 0;
		    }
		}
		else if(TransformationDetails->Spin[i] < 0)
		{
		    TransformationDetails->Spin[i] += TransformationDetails->SpinTarget[i].Acceleration;
		    if(TransformationDetails->Spin[i] < TransformationDetails->SpinTarget[i].Speed)
		    {
			TransformationDetails->Spin[i] = TransformationDetails->SpinTarget[i].Speed;
			TransformationDetails->SpinTarget[i].Acceleration = 0;
		    }
		}
	    }
	}
	TransformationDetails->Rotation[i] += TransformationDetails->Spin[i];
    }
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	UpdateTransformationDetails(&Object->Children[i],&TransformationDetails->Children[i],TimeStep);
    }
}

void RenderObject3d(Object3d * Object,Object3dTransformationDetails * TransformationDetails,GLuint ModelMatrixLocation, uint8_t * PaletteData, GLuint DebugAxisBuffer, int32_t Animate,int32_t Side,Matrix ParentMatrix=Matrix())
{
    if((TransformationDetails->Flags & OBJECT3D_FLAG_HIDE))
	return;
    if(Animate)
    {
	Object->TextureOffset++;
    }
//Object->TextureOffset = 3;
    Object3dRenderingPrep(Object, PaletteData,Side);
    //TODO(Christof): Actually make use of TransformationDetails

    Matrix CurrentMatrix;
    CurrentMatrix.Rotate(0,1,0, TransformationDetails->Rotation[1]);
    CurrentMatrix.Rotate(1,0,0, TransformationDetails->Rotation[0]);
    CurrentMatrix.Rotate(0,0,1, TransformationDetails->Rotation[2]);
    
    CurrentMatrix.Move(Object->Position.X,Object->Position.Y,Object->Position.Z);

   CurrentMatrix = ParentMatrix * CurrentMatrix ;
//       CurrentMatrix =  CurrentMatrix *ParentMatrix ;
    
    
    CurrentMatrix.Move(TransformationDetails->Movement[0],TransformationDetails->Movement[1],TransformationDetails->Movement[2]);

    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	RenderObject3d(&Object->Children[i],&TransformationDetails->Children[i],ModelMatrixLocation,PaletteData,DebugAxisBuffer,Animate,Side,CurrentMatrix);
    }


    CurrentMatrix.Upload(ModelMatrixLocation);
    glBindVertexArray(Object->VertexBuffer);
    glDrawArrays(GL_TRIANGLES, 0, Object->NumTriangles*3);
    GLenum ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed to render : %s",gluErrorString(ErrorValue));
    }
    
    glBindVertexArray(Object->LineBuffer);
    glDrawArrays(GL_LINES, 0, Object->NumLines*2);
    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed to render : %s",gluErrorString(ErrorValue));
    }

    #if 0
    //Debug Axis rendering
    glBindVertexArray(DebugAxisBuffer);
    glDrawArrays(GL_LINES, 0, 3*2);
    ErrorValue = glGetError();
    if(ErrorValue!=GL_NO_ERROR)
    {
	LogError("failed to render : %s",gluErrorString(ErrorValue));
    }
    #endif

    

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
    if(header->Always_0 != 0)
    {
	LogWarning("Always 0 is in fact %d in %s", header->Always_0, header->OffsetToObjectName);
    }

    //TODO(Christof): Bounds check
    int NameLength=(int)strlen((char*)Buffer + header->OffsetToObjectName)+1;
    memcpy(Object->Name,Buffer+header->OffsetToObjectName,NameLength);
    Object->Position.X=header->XFromParent*TA_TO_GL_SCALE*TranslationXAxisModModel;
    Object->Position.Y=header->YFromParent*TA_TO_GL_SCALE*TranslationYAxisModModel;
    Object->Position.Z=header->ZFromParent*TA_TO_GL_SCALE*TranslationZAxisModModel;

    FILE_Object3dPrimitive * Primitives=(FILE_Object3dPrimitive*)(Buffer+header->OffsetToPrimitiveArray);
    FILE_Object3dPrimitive * CurrentPrimitive = Primitives;
    Object->NumberOfPrimitives = header->NumberOfPrimitives;
    Object->Primitives = PushArray(GameArena,Object->NumberOfPrimitives,Object3dPrimitive);
    for(int i=0;i<header->NumberOfPrimitives;i++)
    {
	if(CurrentPrimitive->OffsetToTextureName)
	{
	    Object->Primitives[i].Texture = GetTexture((char*)Buffer+CurrentPrimitive->OffsetToTextureName,NextTexture,Textures);
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
	Object->Vertices[i*3+0]=Vertices[i].x*TA_TO_GL_SCALE*TranslationXAxisModModel;
	Object->Vertices[i*3+1]=Vertices[i].y*TA_TO_GL_SCALE*TranslationYAxisModModel;
	Object->Vertices[i*3+2]=Vertices[i].z*TA_TO_GL_SCALE*TranslationZAxisModModel;
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
    {
	glDeleteBuffers(1,&Object->VertexBuffer);	
	glDeleteBuffers(1,&Object->LineBuffer);
	Object->LineBuffer = Object->VertexBuffer = 0;
    }
    for(int i=0;i<Object->NumberOfChildren;i++)
    {
	Unload3DO(&Object->Children[i]);
    }
}
