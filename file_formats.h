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
const u32 OBJECT3D_FLAG_CACHE = 2;
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
const int LIST_ITEM_HEIGHT = 16;

//TDF
const int TDF_MAX_STRING_LENGTH  = 64;

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
const int MAX_INSTRUCTIONS_PER_FRAME = 10000;
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

internal const char * UnitVariableNames[]=
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
//MMAP STUFF
struct MemoryMappedFile
{
    unsigned char * MMapBuffer;
    u64  FileSize;
    u64  ModifiedTime;
#ifdef __WINDOWS__
    HANDLE MMFile;
    HANDLE File;
#else
    s32 File;
    s32 PAD;
#endif
};

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
    s32 TextureWidth, TextureHeight, HeightInSquares, WidthInSquares;
    s32 RequestedHeight, RequestedWidth;
    GLuint Texture;
    TexturePosition FirstFreeTexture;
    s32 PAD;
};

//3DO
struct Position3d
{
    float X,Y,Z;
};

struct Object3d
{
    struct Object3dPrimitive * Primitives;
    char Name[MAX_3DO_NAME_LENGTH];
    Object3d * Children;
    int NumberOfChildren;
    Position3d Position;
    int NumberOfPrimitives;

    int NumberOfVertices;
    GLfloat * Vertices;
    GLuint VertexBuffer;
    int NumTriangles;
    int NumLines;
    GLuint LineBuffer;
    b32 Animates;
    s32 SelectionPrimitive;
    GLuint LineVertexBuffer;
    GLuint TriangleVertexBuffer;
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
    Object3dTransformationDetails * Children;
    RotationDetails RotationTarget[TA_AXIS_NUM];
    MovementDetails MovementTarget[TA_AXIS_NUM];
    SpinDetails SpinTarget[TA_AXIS_NUM];

    r32 Rotation[TA_AXIS_NUM];
    r32 Movement[TA_AXIS_NUM];
    r32 Spin[TA_AXIS_NUM];


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
    s32 PAD;
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
    struct HPIEntry * Entries;
    s32 NumberOfEntries;
    s32 PAD;
};

struct HPIEntry
{
    char * Name;
    struct HPIFile * ContainedInFile;
    union
    {
	HPIFileEntry File;
	HPIDirectoryEntry Directory;
    };
    b32 IsDirectory;
    s32 PAD;
};

struct HPIFile
{
    MemoryMappedFile MMFile;
    HPIDirectoryEntry Root;
    char Name[MAX_HPI_FILE_NAME];
    s32 DecryptionKey;
    s32 PAD;
};

struct HPIFileCollection
{
    //TODO(Christof): track directory(s?) as well
    HPIFile * Files;
    s32 NumberOfFiles;
    s32 PAD;
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
    ShaderProgram * Program;
    GLint ColorLocation, AlphaLocation, PositionLocation, SizeLocation, TextureOffsetLocation;
    s32 PAD;
};

//TAUI

struct TAUIContainer
{
    Texture * Background;//Name is in Panel
    struct TAUIElement * DefaultFocus;
    TAUIElement * Elements;
    s32 NumberOfElements;
    s32 PAD;
};

struct TAUIButton
{
    int StartingFrame;//pulls from status
    int Stages;
    char * Text;// | seperator for multistage buttons, center aligned for simple (single stage) buttons, right aligned otherwise
    b32 Disabled;//pulls from grayedout
    b32 Pressed;
    s32 CurrentStage;
    s32 PAD;
};

struct TAUITextBox
{
    int MaximumCharacters;
};

struct TAUIListBox
{
    char ** ItemStrings;
    s32 NumberOfDisplayableItems, NumberOfItems, DisplayItemIndex;
    s32 SelectedIndex;
};

struct TAUIScrollbar
{
    r32 KnobPosition;
    r32 KnobSize;
    TAUIListBox * ListBox;
    b32 Pressed;
    s32 PAD;
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

enum TAUIElementName
{
    ELEMENT_NAME_UNKNOWN,
    ELEMENT_NAME_SINGLEPLAYER,
    ELEMENT_NAME_MULTIPLAYER,
    ELEMENT_NAME_DEBUGSTRING,
    ELEMENT_NAME_EXIT,
    ELEMENT_NAME_NEW_CAMPAIGN,
    ELEMENT_NAME_LOAD_GAME,
    ELEMENT_NAME_SKIRMISH,
    ELEMENT_NAME_PREVIOUS_MENU,
    ELEMENT_NAME_OPTIONS,
    ELEMENT_NAME_OK,
    ELEMENT_NAME_CANCEL,
    ELEMENT_NAME_LOAD,

    ELEMENT_NAME_INTRO,
    ELEMENT_NAME_CREDITS,
    ELEMENT_NAME_START,
    ELEMENT_NAME_CAMPAIGN,
    ELEMENT_NAME_CAMPAIGN_KNOB,
    ELEMENT_NAME_MISSIONS_KNOB,
    ELEMENT_NAME_MISSIONS,
    ELEMENT_NAME_SIDE_0,
    ELEMENT_NAME_SIDE_1,
    ELEMENT_NAME_SIDE_NAME,
    ELEMENT_NAME_DIFFICULTY,
    ELEMENT_NAME_ARM,
    ELEMENT_NAME_CORE,

    ELEMENT_NAME_GAMES,
    ELEMENT_NAME_SLIDER,
    ELEMENT_NAME_GAME_NAME,
    ELEMENT_NAME_GAME_TYPE,
    ELEMENT_NAME_MISSION,
    ELEMENT_NAME_TIME,
    ELEMENT_NAME_SIDE,
    ELEMENT_NAME_RADAR,
    ELEMENT_NAME_DIFF,
    ELEMENT_NAME_DELETE,
    ELEMENT_NAME_SAVE,
    ELEMENT_NAME_SKIRMISH_GUI,
    ELEMENT_NAME_HEADER,
    ELEMENT_NAME_TEXT,
    ELEMENT_NAME_SELECT_MAP,
    ELEMENT_NAME_START_LOCATION,
    ELEMENT_NAME_COMMANDER_DEATH,
    ELEMENT_NAME_MAPPING,
    ELEMENT_NAME_MAP_NAME,
    ELEMENT_NAME_LINE_OF_SIGHT,
    ELEMENT_NAME_HELP_TEXT,
    ELEMENT_NAME_SOUND,
    ELEMENT_NAME_SPEEDS,
    ELEMENT_NAME_VISUALS,
    ELEMENT_NAME_MUSIC,
    ELEMENT_NAME_RESTORE,
    ELEMENT_NAME_UNDO,
    ELEMENT_NAME_MAIN_MENU_GUI,
    ELEMENT_NAME_ANY_MISSION,
};

struct TAUIElement
{
    char * Name;
    char * Help;
    TAUIElementName ElementName;
    TagType ElementType;
    int Association,X,Y,Width,Height,ColorFore,ColorBack,TextureNumber,FontNumber;
    int Attributes, CommonAttributes;

    b32 Visible;//Pulls from active
    TextureContainer * Textures;
    union
    {
	TAUIButton Button;
	TAUIContainer Container;
	TAUITextBox TextBox;
	TAUIListBox ListBox;
	TAUIScrollbar ScrollBar;
	TAUILabel Label;
	TAUIDynamicImage DynamicImage;
	TAUILabelFont LabelFont;
    } ;
};

//TDF

struct TDFNameValue
{
    char Name[TDF_MAX_STRING_LENGTH];
    char Value[TDF_MAX_STRING_LENGTH];
    TDFNameValue * Next;
};

struct TDFElement
{
    TDFElement * Next;
    TDFElement * Child;
    TDFNameValue * Value;
    char Name[TDF_MAX_STRING_LENGTH];
};

//UnitScripts
struct UnitScript
{
    s32 * ScriptData;
    s32 * FunctionOffsets;
    int NumberOfFunctions;
    int NumberOfPieces;
    char ** FunctionNames;
    char ** PieceNames;
    int ScriptDataSize;

    s32 NumberOfStatics;
    //TODO(Christof): Make this part of the unit struct?
};

struct ScriptState
{
    //TODO(Christof): determine if stack can contain floats? some docs seem to indicate they should?
    s32 StackSize;
    s32 NumberOfLocalVariables;
    s32 * StackData;
    s32 StackStorage[UNIT_SCRIPT_MAX_STACK_SIZE];
    s32 LocalVariables[UNIT_SCRIPT_MAX_STACK_SIZE];//NOTE(Christof): function parameters go at the beginning
    s32 NumberOfStaticVariables;
    s32 ProgramCounter;
    s32 * StaticVariables;
    Block BlockedOn;
    s32 BlockTime;//NOTE(Christof): this is in milliseconds
    s32 BlockedOnPiece;
    s32 BlockedOnAxis;
    s32 SignalMask;
    s32 ScriptNumber;
    ScriptState * ReturnTo;
    struct Object3dTransformationDetails * TransformationDetails;
    s32 NumberOfParameters;
    s32 PAD;
};

struct ScriptStatePool
{
    ScriptState Scripts[SCRIPT_POOL_SIZE];
    int NumberOfScripts;
    s32 StaticVariables[UNIT_SCRIPT_MAX_STACK_SIZE];
    s32 PAD;
};

//Units
enum UnitCommandType
{
    UNIT_COMMAND_MOVE,
    UNIT_COMMAND_PATROL,
    UNIT_COMMAND_ATTACK,
    UNIT_COMMAND_WAIT,
    UNIT_COMMAND_BUILD,
    UNIT_COMMAND_WAIT_ATTACKED
};

struct UnitType
{
    Object3d Model;
    UnitScript Script;
    UnitDetails Details;
};

const s32 MAX_UNIT_TYPES = 1024;
struct UnitTypeList
{
    UnitType Types[MAX_UNIT_TYPES];
    s32 NumberOfUnitTypes;
};

struct PatrolArguments
{
    s32 X,Y,NextOffset;
};

struct MoveArguments
{
    s32 X,Y;
};

const s32 MAX_IDENT=16;
struct AttackArguments
{
    char Name[MAX_IDENT];
};

struct WaitArguments
{
    r32 Seconds;
};

struct BuildArguments
{
    UnitType * Type;
    union
    {
	struct
	{
	s32 X,Y;
	};
	s32 Number;
    };
};

struct WaitAttackedArguments
{
    char Name[MAX_IDENT];
};

struct UnitCommand
{
    union
    {
	PatrolArguments PatrolArguments;
	MoveArguments MoveArguments;
	AttackArguments AttackArguments;
	WaitArguments WaitArguments;
	WaitAttackedArguments WaitAttackedArguments;
	BuildArguments BuildArguments;
    };
    UnitCommandType Command;
};


const s32 MAX_UNIT_COMMANDS = 32;
struct Unit
{
    UnitType * Type;
    ScriptStatePool ScriptPool;
    UnitSide Side;
    r32 Health, BuildPercent;
    s32 NumberOfCommands;
    UnitCommand Commands[MAX_UNIT_COMMANDS];
    char Name[MAX_IDENT];
};

//TNT

const s32 MAX_INITIAL_UNITS = 128;
struct TAMap
{
    GLuint MapVertexBuffer, MapTexture, MinimapTexture,VertexBuffer;
    int NumTriangles;
    s32 MinimapHeight,MinimapWidth;
    s32 TidalStrength, SolarStrength, MinWindSpeed, MaxWindSpeed, Gravity, WaterDamage;
};


//Campaigns

struct Mission
{
    char * MissionFile;
    char * MissionName;
};

struct Campaign
{
    char * CampaignName;
    Mission * Missions;
    s32 NumberOfMissions;
    s32 PAD;
};

const s32 MAX_CAMPAIGNS = 16;

struct CampaignList
{
    s32 NumberOfARMCampaigns;
    s32 NumberOfCORECampaigns;
    Campaign ARMCampaigns[MAX_CAMPAIGNS];
    Campaign CORECampaigns[MAX_CAMPAIGNS];
};
