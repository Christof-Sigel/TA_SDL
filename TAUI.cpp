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


#define TAUI_ATTRIBUTE_SCROLLBAR_HORIZONTAL 1
#define TAUI_ATTRIBUTE_SCROLLBAR_VERTICAL 2

struct TAUIElement
{
    char * Name;
    TagType ElementType;
    int Association,X,Y,Width,Height,ColorFore,ColorBack,TextureNumber,FontNumber;
    int Attributes, CommonAttributes;
    char * Help;
    uint8_t Visible;//Pulls from active
    void * Details;
};

struct TAUIContainer
{
    Texture * Background;//Name is in Panel
    TAUIElement * DefaultFocus;
};

struct TAUIButton
{
    int StartingFrame;//pulls from status
    int Stages;
    char * Text;// | seperator for multistage buttons, center aligned for simple (single stage) buttons, right aligned otherwise
    uint8_t Disabled;//pulls from grayedout
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
    uint8_t DisaplySelectionRectangle;//Puller from hotornot
};

struct TAULabelFont
{
    FNTFont * Font;//from filename
};



TAUIElement LoadGUIFromBuffer(char * Buffer)
{

    return{0};
}


void LoadCommonUITextures(GameState * CurrentGameState)
{
    SetupTextureContainer(CurrentGameState->CommonGUITextures, COMMONUI_TEXTURE_WIDTH, COMMONUI_TEXTURE_HEIGHT, COMMONUI_MAX_TEXTURES, &CurrentGameState->GameArena);
        if(!CurrentGameState->PaletteLoaded)
    {
	LoadPalette(CurrentGameState);
    }
    HPIEntry CommonUI = FindEntryInAllFiles("anims/commonGUI.GAF", CurrentGameState);
    if(CommonUI.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load hatfont12.gaf");
    }
    else if(!CommonUI.Name)
    {
	LogError("Failed to get commonGUI.gaf");
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&CommonUI, CurrentGameState->CommonGUITextures, &CurrentGameState->TempArena, CurrentGameState->PaletteData);
    }

}
