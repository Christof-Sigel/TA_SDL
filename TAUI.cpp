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

const int FILE_UI_MAX_STRING_LENGTH  = 32;

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
    int NumberOfElements;
    TAUIElement * Elements;
    TextureContainer * Textures;
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

char * GetStringValue(FILE_UIElement * Element, const char * Name)
{
    FILE_UINameValue * NameValue = Element->Value;
    while(NameValue)
    {
	if(CaseInsensitiveMatch(Name, NameValue->Name))
	    return NameValue->Value;
	NameValue = NameValue->Next;
    }
    return 0;
}

int GetIntValue(FILE_UIElement * Element, const char * Name)
{
    char * Value=GetStringValue(Element,Name);
    if(Value)
	return atoi(Value);
    return -1;
}
    
float GetFloat(FILE_UIElement * Element, const char * Name)
{
    char * Value= GetStringValue(Element,Name);
    if(Value)
	return (float)atof(Value);
    return -1;
}

FILE_UIElement * GetSubElement(FILE_UIElement * Root, const char * Name)
{
    FILE_UIElement * element = Root->Child;
    while(element)
    {
	if(CaseInsensitiveMatch(Name, element->Name))
	    return element;
	element=element->Next;
    }
    return 0;
}

FILE_UIElement * GetNthElement(FILE_UIElement * Start, int N)
{
    if(N ==0 || !Start)
	return Start;
    return GetNthElement(Start, N-1);
}

int CountElements(FILE_UIElement * First)
{
    int count = 0;
    while(First)
    {
	First = First->Next;
	count++;
    }
    return count;
}

FILE_UINameValue * LoadUINameValueFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    char * Buffer = *InBuffer;
    char * Start=Buffer;
    while(*Buffer != '=' && *Buffer != ' ' && *Buffer != '\t' && *Buffer != '\r' && *Buffer!='\n' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    char sep = *Buffer;
    *Buffer =0;
    FILE_UINameValue * Result = PushStruct(Arena, FILE_UINameValue);
    *Result={};
    strncpy(Result->Name, Start, FILE_UI_MAX_STRING_LENGTH);
    Buffer++;
    if(sep != '=')
    {
	while((*Buffer == '=' || *Buffer != ' ' || *Buffer != '\t' || *Buffer != '\r' || *Buffer!='\n')
	      && Buffer <=End) {  Buffer++;  }
	if(Buffer >= End){	return 0; }
    }
    Start = Buffer;
    while(*Buffer != ';' && Buffer <=End) {  Buffer++;  }
    if(Buffer >= End){	return 0; }
    *Buffer =0;
    strncpy(Result->Value, Start, FILE_UI_MAX_STRING_LENGTH);
    Buffer++;
    
    *InBuffer = Buffer;
    return Result;
}

FILE_UIElement * LoadUIElementFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    char * Buffer = *InBuffer;
    while(*Buffer != '[' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    char * Start = Buffer+1;
    while(*Buffer != ']' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    *Buffer =0;
    
    FILE_UIElement * Result = PushStruct(Arena, FILE_UIElement);
    *Result={};
    strncpy(Result->Name, Start, FILE_UI_MAX_STRING_LENGTH);
    while(*Buffer != '{' && Buffer <=End) {  Buffer++;  }
    if(Buffer == End){	return 0; }
    Buffer++;

    while(Buffer<=End)
    {
	while((*Buffer == ' ' || *Buffer == '\t' || *Buffer =='\r' || *Buffer == '\n' )&& Buffer <=End) {  Buffer++;  }
	if(Buffer == End){	return 0; }
	if(*Buffer =='}')
	{
	    Buffer++;
	    *InBuffer = Buffer;
	    return Result;
	}
	else if(*Buffer == '[')
	{
	    if(Result->Child)
	    {
		FILE_UIElement * Child = Result->Child;
		while(Child->Next)
		{
		    Child=Child->Next;
		}
		Child->Next=LoadUIElementFromBuffer(&Buffer,End, Arena);
	    }
	    else
	    {
		Result->Child = LoadUIElementFromBuffer(&Buffer, End, Arena);
	    }
	}
	else
	{
	    if(Result->Value)
	    {
		FILE_UINameValue * Value = Result->Value;
		while(Value->Next)
		{
		    Value=Value->Next;
		}
		Value->Next = LoadUINameValueFromBuffer(&Buffer, End, Arena);
	    }
	    else
	    {
		Result->Value=LoadUINameValueFromBuffer(&Buffer, End, Arena);
	    }
	}
	    
    }
    *InBuffer = Buffer;
    return 0;
}

FILE_UIElement * LoadUIElementsFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
{
    FILE_UIElement * First = LoadUIElementFromBuffer(InBuffer, End, Arena);
    FILE_UIElement * Last = First;
    while(*InBuffer <End)
    {
	Last->Next = LoadUIElementFromBuffer(InBuffer, End, Arena);
	if(Last->Next)
	{
	    Last = Last->Next;
	}
    }
    return First;
}

void LoadElementFromTree(TAUIElement * Element, FILE_UIElement * Tree, MemoryArena * Arena, TextureContainer * Textures, GameState * CurrentGameState)
{
    FILE_UIElement * Common = GetSubElement(Tree,"common");
    Element->ElementType = (TagType)GetIntValue(Common, "ID");

    Element->Association = GetIntValue(Common, "assoc");
    Element->X = GetIntValue(Common, "xpos");
    Element->Y = GetIntValue(Common, "ypos");
    Element->Width = GetIntValue(Common, "width");
    Element->Height = GetIntValue(Common, "height");
    Element->Attributes = GetIntValue(Common, "attribs");
    Element->ColorFore = GetIntValue(Common, "colorf");
    Element->ColorBack = GetIntValue(Common, "colorb");
    Element->TextureNumber = GetIntValue(Common, "texturenumber");
    Element->FontNumber = GetIntValue(Common, "fontnumber");
    Element->CommonAttributes = GetIntValue(Common, "commonattribs");
    Element->Visible = GetIntValue(Common, "active");

    char * Name = GetStringValue(Common, "name");
    int len = strlen(Name);
    Element->Name = PushArray(Arena, len+1, char);
    if(Element->Name)
    {
	memcpy(Element->Name, Name, len);
	Element->Name[len]=0;
    }
       
    char * Help = GetStringValue(Common, "help");

    len = strlen(Help);
    Element->Help = PushArray(Arena, len+1, char);
    if(Element->Help)
    {
	memcpy(Element->Help, Help, len);
	Element->Help[len]=0;
    }

    switch(Element->ElementType)
    {
    case TAG_UI_CONTAINER:
    {
	TAUIContainer * Container = PushStruct(Arena, TAUIContainer);
	Element->Details = Container;
	if(CaseInsensitiveMatch(Name, "Mainmenu.gui"))
	{
	    Container->Background = AddPCXToTextureContainer(Textures,"bitmaps/FrontEndX.pcx", CurrentGameState);
	}
	else
	{
	    Container->Background = GetTexture(Name, Textures);
	}
    }
    break;
    case TAG_BUTTON:
    {
	TAUIButton * Button = PushStruct(Arena, TAUIButton);
	Element->Details = Button;
	Button->StartingFrame = GetIntValue(Tree, "status");
	Button->Stages = GetIntValue(Tree, "Stages");
	char * Text = GetStringValue(Tree, "text");
	int len = strlen(Text);
	Button->Text = PushArray(Arena, len+1, char);
	if(Button->Text)
	{
	    memcpy(Button->Text, Text, len);
	    Button->Text[len]=0;
	}

	Button->Disabled = GetIntValue(Tree, "grayedout");
    }
    break;
    case TAG_TEXTFIELD:
    {
	TAUITextBox * TextBox = PushStruct(Arena,TAUITextBox);
	Element->Details = TextBox;
	TextBox->MaximumCharacters = GetIntValue(Tree, "maxchars");
    }
    break;
    case TAG_SCROLLBAR:
    {
	TAUIScrollbar * ScrollBar = PushStruct(Arena, TAUIScrollbar);
	Element->Details = ScrollBar;
	ScrollBar->Maximum = GetIntValue(Tree, "range");
	ScrollBar->Position = GetIntValue(Tree, "knobpos");
	ScrollBar->KnobSize = GetIntValue(Tree, "knobsize");
    }
    break;
    //TODO(Christof): handle all UI elements
    }
   
    
}



TAUIElement * LoadGUIFromBuffer(char * Buffer, char * End, MemoryArena * Arena, MemoryArena * TempArena, char * FileName, GameState * CurrentGameState)
{
    TextureContainer * Textures =PushStruct(Arena, TextureContainer);
    SetupTextureContainer(Textures, 1024,1024, 40, Arena);

    
    int len=snprintf(0,0,"anims/%s",FileName)+1;
    STACK_ARRAY(GafFileName,len,char);
    snprintf(GafFileName,len,"anims/%s",FileName);
    GafFileName[len-4]='g';
    GafFileName[len-3]='a';
    GafFileName[len-2]='f';
    
    HPIEntry UITextures = FindEntryInAllFiles(GafFileName, CurrentGameState);
    if(UITextures.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load %s", GafFileName);
    }
    else if(!UITextures.Name)
    {
	LogError("Failed to load %s", GafFileName);
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&UITextures, Textures, TempArena, CurrentGameState->PaletteData);
    }
    MemoryArena * UIElementsArena = PushSubArena(TempArena, 16 * 1024);
    FILE_UIElement * First = LoadUIElementsFromBuffer(&Buffer, End, UIElementsArena);

    if(GetIntValue(GetSubElement(First,"common"),"ID") != 0)
    {
	LogError("First UI element is not a container (%d)!",GetIntValue(GetSubElement(First,"common"),"ID"));
	return 0;
    }

    TAUIElement * Container = PushStruct(Arena, TAUIElement);
    LoadElementFromTree(Container, First, Arena, Textures, CurrentGameState);

    TAUIContainer * ContainerDetails = (TAUIContainer *)Container->Details;
    ContainerDetails->NumberOfElements = CountElements(First)-1;
    ContainerDetails->Elements = PushArray(Arena, ContainerDetails->NumberOfElements, TAUIElement);
    ContainerDetails->Textures = Textures;

    for(int i=0;i<ContainerDetails->NumberOfElements;i++)
    {
	LoadElementFromTree(&ContainerDetails->Elements[i], GetNthElement(First, i+1), Arena, Textures, CurrentGameState);
    }

  


    
    PopSubArena(TempArena, UIElementsArena);
    return Container;
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
