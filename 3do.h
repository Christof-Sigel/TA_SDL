const int32_t TranslationXAxisModModel = 1;
const int32_t TranslationYAxisModModel = 1;
const int32_t TranslationZAxisModModel = -1;

const int32_t TranslationXAxisModScript = -1;
const int32_t TranslationYAxisModScript = 1;
const int32_t TranslationZAxisModScript = 1;

const int32_t RotationXAxisMod = 1;
const int32_t RotationYAxisMod = 1;
const int32_t RotationZAxisMod = -1;




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
