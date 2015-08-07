////////// Defines ////////
//Textures
const s32 GAF_IDVERSION=0x00010100;
const int PIXELS_PER_SQUARE_SIDE = 8;

//3DO
const s32 TranslationXAxisModModel = 1;
const s32 TranslationYAxisModModel = 1;
const s32 TranslationZAxisModModel = -1;

const s32 TranslationXAxisModScript = -1;
const s32 TranslationYAxisModScript = 1;
const s32 TranslationZAxisModScript = 1;

const s32 RotationXAxisMod = 1;
const s32 RotationYAxisMod = 1;
const s32 RotationZAxisMod = -1;

const u32 OBJECT3D_FLAG_HIDE = 1;
const u32 OBJECT3D_FLAG_DONT_CACHE = 2;
const u32 OBJECT3D_FLAG_DONT_SHADE = 4;

const s32 MAX_3DO_NAME_LENGTH = 32;

enum
{
    TA_AXIS_X,
    TA_AXIS_Y,
    TA_AXIS_Z,
    TA_AXIS_NUM
};

//FBI
const s32 MAX_UNIT_DETAILS = 128;

enum UnitSide
{
    SIDE_ARM,
    SIDE_CORE,
    SIDE_UNKNOWN
};

enum
{
    UNIT_CATEGORY_ARM=0,
    UNIT_CATEGORY_KBOT,
    UNIT_CATEGORY_LEVEL2,
    UNIT_CATEGORY_CONSTRUCTOR,
    UNIT_CATEGORY_NO_WEAPON,//4
    UNIT_CATEGORY_NOT_AIR,
    UNIT_CATEGORY_NOT_SUB,
    UNIT_CATEGORY_CTRL_B,
    UNIT_CATEGORY_SHIP,//8
    UNIT_CATEGORY_LEVEL1,
    UNIT_CATEGORY_LEVEL3,
    UNIT_CATEGORY_TANK,
    UNIT_CATEGORY_WEAPON,//12
    UNIT_CATEGORY_CTRL_W,
    UNIT_CATEGORY_SPECIAL,
    UNIT_CATEGORY_SONAR,
    UNIT_CATEGORY_CORE,//16
    UNIT_CATEGORY_LEVEL10,
    UNIT_CATEGORY_CTRL_C,
    UNIT_CATEGORY_ENERGY,
    UNIT_CATEGORY_METAL, //20
    UNIT_CATEGORY_PARAL,
    UNIT_CATEGORY_COMMANDER,
    UNIT_CATEGORY_CARRY,
    UNIT_CATEGORY_CTRL_F,//24
    UNIT_CATEGORY_PLANT,
    UNIT_CATEGORY_RAD,
    UNIT_CATEGORY_TORP,
    UNIT_CATEGORY_CTRL_V,//28
    UNIT_CATEGORY_VTOL,
    UNIT_CATEGORY_STRATEGIC,
    UNIT_CATEGORY_DEFENSIVE,//32
    UNIT_CATEGORY_JAM,
    UNIT_CATEGORY_REPAIR_PAD,
    UNIT_CATEGORY_ANTI_SUB,
    UNIT_CATEGORY_STEALTH,//36
    UNIT_CATEGORY_BOMB,
    UNIT_CATEGORY_PHIB,
    UNIT_CATEGORY_UNDERWATER,
    UNIT_CATEGORY_KAMIKAZE//40
};

//HPI
const s32 SAVE_MARKER= 'B' << 0 | 'A' <<8 | 'N' << 16 | 'K' <<24;
const s32 HPI_MARKER= 'H' << 0 | 'A' <<8 | 'P' <<16 | 'I' << 24;
const s32 CHUNK_MARKER = 'S' << 0 | 'Q' <<8 | 'S' <<16 | 'H' << 24;
const s32 CHUNK_SIZE = 65536;

const s32 MAX_HPI_FILE_NAME = 32;

enum Compression_Type
{
    COMPRESSION_NONE=0,
    COMPRESSION_LZ77,
    COMPRESSION_ZLIB
};

//Font

//TAUI
enum TagType
{
    TAG_UI_CONTAINER =0,
    TAG_BUTTON,
    TAG_LISTBOX,//No extra details
    TAG_TEXTFIELD,
    TAG_SCROLLBAR,
    TAG_LABEL,
    TAG_DYNAMIC_IMAGE,
    TAG_LABEL_FONT,
    TAG_IMAGE = 12//No extra details
};

const int TAUI_ATTRIBUTE_SCROLLBAR_HORIZONTAL = 1;
const int TAUI_ATTRIBUTE_SCROLLBAR_VERTICAL = 2;
const int FILE_UI_MAX_STRING_LENGTH  = 32;

//UnitScripts
enum Block
{
    BLOCK_INIT,
    BLOCK_NOT_BLOCKED,
    BLOCK_MOVE,
    BLOCK_TURN,
    BLOCK_SLEEP,
    BLOCK_DONE,
    BLOCK_SCRIPT
};

const int SCRIPT_NAME_STORAGE_SIZE=64;
const int UNIT_SCRIPT_MAX_STACK_SIZE = 128;
const int UNIT_SCRIPT_MAX_LOCAL_VARS = 64;
const int MAX_INSTRUCTIONS_PER_FRAME = 2000;
const int SCRIPT_POOL_SIZE = 64;

enum UnitVariable
{
    UNIT_VAR_ACTIVATION =1,
    UNIT_VAR_STANDING_MOVE_ORDERS,
    UNIT_VAR_STANDING_FIRE_ORDERS,
    UNIT_VAR_HEALTH,
    UNIT_VAR_IN_BUILD_STANCE,
    UNIT_VAR_BUSY,
    UNIT_VAR_PIECE_XZ,
    UNIT_VAR_PIECE_Y,
    UNIT_VAR_UNIT_XZ,
    UNIT_VAR_UNIT_Y,
    UNIT_VAR_UNIT_HEIGHT,
    UNIT_VAR_XZ_ATAN,
    UNIT_VAR_XZ_HYPOT,
    UNIT_VAR_ATAN,
    UNIT_VAR_HYPOT,
    UNIT_VAR_GROUND_HEIGHT,
    UNIT_VAR_BUILD_PERCENT_LEFT,
    UNIT_VAR_YARD_OPEN,
    UNIT_VAR_BUGGER_OFF,
    UNIT_VAR_ARMORED
};

char * UnitVariableNames[]=
{
    "NONE",
    "UNIT_VAR_ACTIVATION",
    "UNIT_VAR_STANDING_MOVE_ORDERS",
    "UNIT_VAR_STANDING_FIRE_ORDERS",
    "UNIT_VAR_HEALTH",
    "UNIT_VAR_IN_BUILD_STANCE",
    "UNIT_VAR_BUSY",
    "UNIT_VAR_PIECE_XZ",
    "UNIT_VAR_PIECE_Y",
    "UNIT_VAR_UNIT_XZ",
    "UNIT_VAR_UNIT_Y",
    "UNIT_VAR_UNIT_HEIGHT",
    "UNIT_VAR_XZ_ATAN",
    "UNIT_VAR_XZ_HYPOT",
    "UNIT_VAR_ATAN",
    "UNIT_VAR_HYPOT",
    "UNIT_VAR_GROUND_HEIGHT",
    "UNIT_VAR_BUILD_PERCENT_LEFT",
    "UNIT_VAR_YARD_OPEN",
    "UNIT_VAR_BUGGER_OFF",
    "UNIT_VAR_ARMORED"};

//TNT
const int TNT_HEADER_ID=0x2000;


////////// Structs ////////

//Textures
struct TexturePosition
{
    int X;
    int Y;
};

struct Texture
{
    char Name[32];
    s32 FrameNumber;
    r32 U,V;
    r32 Width,Height;
    s32 X, Y;
    s32 NumberOfFrames;
};

struct TextureContainer
{
    Texture * Textures;
    int MaximumTextures;
    int NumberOfTextures;
    u8 * FreeSquares;
    int TextureWidth, TextureHeight, HeightInSquares, WidthInSquares;
    GLuint Texture;
    TexturePosition FirstFreeTexture;
};

//3DO
struct Position3d
{
    float X,Y,Z;
};

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
    b32 Animates;
    s32 SelectionPrimitive;
};

struct Object3dPrimitive
{
    Texture * Texture;
    int NumberOfVertices;
    int ColorIndex;
    int * VertexIndexes;
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

struct Object3dTransformationDetails
{
    RotationDetails RotationTarget[TA_AXIS_NUM];
    MovementDetails MovementTarget[TA_AXIS_NUM];
    SpinDetails SpinTarget[TA_AXIS_NUM];
    
    float Rotation[TA_AXIS_NUM];
    float Movement[TA_AXIS_NUM];
    float Spin[TA_AXIS_NUM];

    Object3dTransformationDetails * Children;
    int TextureOffset;
    GLuint TextureCoordBuffer;

    u32 Flags;
};

//FBI
struct UnitKeyValue
{
    char * Name, *Value;
};

struct UnitDetails
{
    UnitKeyValue Details[MAX_UNIT_DETAILS];
    int DetailsSize;
    int GetInt(const char * Name);
    
    float GetFloat(const char * Name);
    char * GetString(const char * Name);
    s64  GetUnitCategories();
    UnitSide GetSide();
};

//HPI
struct HPIFileEntry
{
    int Offset;
    int FileSize;
    Compression_Type Compression;
};

struct HPIDirectoryEntry
{
    int NumberOfEntries;
    struct HPIEntry * Entries;
};

struct HPIEntry
{
    char * Name;
    b32 IsDirectory;
    struct HPIFile * ContainedInFile;
    union
    {
	HPIFileEntry File;
	HPIDirectoryEntry Directory;
    };
};

struct HPIFile
{
    MemoryMappedFile MMFile;
    HPIDirectoryEntry Root;
    s32 DecryptionKey;
    char Name[MAX_HPI_FILE_NAME];
};

struct HPIFileCollection
{
    //TODO(Christof): track directory(s?) as well
    int NumberOfFiles;
    HPIFile * Files;
};

//Font
struct FNTGlyph
{
    int Width;
    float U;
    float TextureWidth;
};

struct FNTFont
{
    int Height;
    GLuint Texture;
    FNTGlyph Characters[256];
    char Name[64];
};

struct FontContainer
{
    FNTFont Fonts[MAX_TA_FONT_NUMBER];
    s32 NumberOfFonts;
    char FontNames[MAX_TA_FONT_NUMBER][64];
};

struct FontShaderDetails
{
    GLuint ColorLocation, AlphaLocation, PositionLocation, SizeLocation, TextureOffsetLocation;
    ShaderProgram * Program;
};

//TAUI

struct TAUIContainer
{
    Texture * Background;//Name is in Panel
    struct TAUIElement * DefaultFocus;
    int NumberOfElements;
    TAUIElement * Elements;
   
};

struct TAUIButton
{
    int StartingFrame;//pulls from status
    int Stages;
    char * Text;// | seperator for multistage buttons, center aligned for simple (single stage) buttons, right aligned otherwise
    b8 Disabled;//pulls from grayedout
    s32 CurrentStatus;
};

struct TAUITextBox
{
    int MaximumCharacters;
};

struct TAUIScrollbar
{
    int Maximum;
    int Position;
    int KnobSize;
};

struct TAUILabel
{
    char * Text;
    TAUIButton * Link;
};

struct TAUIDynamicImage
{
    b8 DisplaySelectionRectangle;//Puller from hotornot
};

struct TAUILabelFont
{
    FNTFont * Font;//from filename
};

struct TAUIElement
{
    char * Name;
    TagType ElementType;
    int Association,X,Y,Width,Height,ColorFore,ColorBack,TextureNumber,FontNumber;
    int Attributes, CommonAttributes;
    char * Help;
    b32 Visible;//Pulls from active
    TextureContainer * Textures;
    union
    {
	TAUIButton Button;
	TAUIContainer Container;
	TAUITextBox TextBox;
	TAUIScrollbar ScrollBar;
	TAUILabel Label;
	TAUIDynamicImage DynamicImage;
	TAUILabelFont LabelFont;
    } ;
};


struct FILE_UINameValue
{
    char Name[FILE_UI_MAX_STRING_LENGTH];
    char Value[FILE_UI_MAX_STRING_LENGTH];
    FILE_UINameValue * Next;
};

struct FILE_UIElement
{
    FILE_UIElement * Next;
    FILE_UIElement * Child;
    FILE_UINameValue * Value;
    char Name[FILE_UI_MAX_STRING_LENGTH];
};

//UnitScripts
struct UnitScript
{
    int NumberOfFunctions;
    int NumberOfPieces;
    char ** FunctionNames;
    char ** PieceNames;
    int ScriptDataSize;
    s32 * ScriptData;
    s32 * FunctionOffsets;
    s32 NumberOfStatics;
    //TODO(Christof): Make this part of the unit struct?
};

struct ScriptState
{
    //TODO(Christof): determine if stack can contain floats? some docs seem to indicate they should?
    int StackSize;
    s32 Stack[UNIT_SCRIPT_MAX_STACK_SIZE];
    int NumberOfLocalVariables;
    s32 LocalVariables[UNIT_SCRIPT_MAX_STACK_SIZE];//NOTE(Christof): function parameters go at the beginning
    int NumberOfStaticVariables;
    s32 * StaticVariables;
    int ProgramCounter;
    Block BlockedOn;
    int BlockTime;//NOTE(Christof): this is in milliseconds
    int BlockedOnPiece;
    int BlockedOnAxis;
    u32 SignalMask;
    ScriptState * ReturnTo;
    int ScriptNumber;
    int NumberOfParameters;
    struct Object3dTransformationDetails * TransformationDetails;
    u32 CurrentInstructionCount;
};

struct ScriptStatePool
{
    ScriptState Scripts[SCRIPT_POOL_SIZE];
    int NumberOfScripts;
    s32 StaticVariables[UNIT_SCRIPT_MAX_STACK_SIZE];
};

//TNT
struct TAMap
{
    GLuint MapVertexBuffer, MapTexture, MinimapTexture;
    int NumTriangles;
    void Render(ShaderProgram * MapShader)
    {
	glUseProgram(MapShader->ProgramID);
	glBindTexture(GL_TEXTURE_2D,MapTexture);
	glBindVertexArray(MapVertexBuffer);
	glDrawArrays(GL_TRIANGLES, 0, NumTriangles*3);
	
    }

    void RenderMiniMap()
    {
	//TODO(Christof): Render into TAUI elements
    }
};
