

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
    return GetNthElement(Start->Next, N-1);
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

void LoadElementFromTree(TAUIElement * Element, FILE_UIElement * Tree, MemoryArena * Arena, TextureContainer * Textures, MemoryArena * TempArena, HPIFileCollection * GlobalArchiveCollection, FontContainer * FontContainer)
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
    if(Help)
    {
	len = strlen(Help);
	Element->Help = PushArray(Arena, len+1, char);
	if(Element->Help)
	{
	    memcpy(Element->Help, Help, len);
	    Element->Help[len]=0;
	}
    }

    switch(Element->ElementType)
    {
    case TAG_UI_CONTAINER:
    {
	if(CaseInsensitiveMatch(Name, "Mainmenu.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/FrontEndX.pcx", GlobalArchiveCollection, TempArena);
	}
	else if(CaseInsensitiveMatch(Name, "header"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/GameSettings.pcx", GlobalArchiveCollection, TempArena);
	}
	else
	{
	    Element->Container.Background = GetTexture(Name, Textures);
	}
    }
    break;
    case TAG_BUTTON:
    {
	Element->Button.StartingFrame = GetIntValue(Tree, "status");
	Element->Button.Stages = GetIntValue(Tree, "Stages");
	char * Text = GetStringValue(Tree, "text");
	if(Text)
	{
	    int len = strlen(Text);
	    Element->Button.Text = PushArray(Arena, len+1, char);
	    if(Element->Button.Text)
	    {
		memcpy(Element->Button.Text, Text, len);
		Element->Button.Text[len]=0;
	    }
	}
	Element->Button.Disabled = GetIntValue(Tree, "grayedout");
    }
    break;
    case TAG_TEXTFIELD:
    {
	Element->TextBox.MaximumCharacters = GetIntValue(Tree, "maxchars");
    }
    break;
    case TAG_SCROLLBAR:
    {
	Element->ScrollBar.Maximum = GetIntValue(Tree, "range");
	Element->ScrollBar.Position = GetIntValue(Tree, "knobpos");
	Element->ScrollBar.KnobSize = GetIntValue(Tree, "knobsize");
    }
    break;
    case TAG_LABEL:
    {
	//TODO(Christof): Load Linked button
	char * Text = GetStringValue(Tree, "text");
	if(Text)
	{
	    int len = strlen(Text);
	    Element->Label.Text = PushArray(Arena, len+1, char);
	    if(Element->Label.Text)
	    {
		memcpy(Element->Label.Text, Text, len);
		Element->Label.Text[len]=0;
	    }
	}
    }
	break;
    case TAG_DYNAMIC_IMAGE:
    {
	Element->DynamicImage.DisplaySelectionRectangle = GetIntValue(Tree, "hotornot");
    }
    break;
    case TAG_LABEL_FONT:
    {
	char * FileName = GetStringValue(Tree, "filename");
	Element->LabelFont.Font=GetFont(FontContainer,FileName, GlobalArchiveCollection, TempArena);
    }
    break;
    case TAG_LISTBOX:
    case TAG_IMAGE:
	//NOTE(Christof): no extra loading required for these
	break;
    default:
	Assert(!"NUH UH!");
	break;
	
    }
   
    
}

TAUIElement LoadGUIFromBuffer(char * Buffer, char * End, MemoryArena * Arena, MemoryArena * TempArena, char * FileName, HPIFileCollection * GlobalArchiveCollection, u8 * PaletteData, FontContainer * FontContainer)
{
    TextureContainer * Textures =PushStruct(Arena, TextureContainer);
    //TODO(Christof): Better texture memory allocation needed, perhaps on Temp and only full size when initially loading gafs?
    SetupTextureContainer(Textures, 1024,1024, 40, Arena);

    
    int len=snprintf(0,0,"anims/%s",FileName)+1;
    STACK_ARRAY(GafFileName,len,char);
    snprintf(GafFileName,len,"anims/%s",FileName);
    GafFileName[len-4]='g';
    GafFileName[len-3]='a';
    GafFileName[len-2]='f';
    
    HPIEntry UITextures = FindEntryInAllFiles(GafFileName, GlobalArchiveCollection, TempArena);
    if(UITextures.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load %s", GafFileName);
    }
    else if(!UITextures.Name)
    {
	//NOTE(Christof): just silently fail here, most guis don't have a .gaf
    }
    else
    {
	LoadAllTexturesFromHPIEntry(&UITextures, Textures, TempArena, PaletteData);
    }
    MemoryArena * UIElementsArena = PushSubArena(TempArena, 64*1024);
    FILE_UIElement * First = LoadUIElementsFromBuffer(&Buffer, End, UIElementsArena);
    TAUIElement Container ={};
    if(First)
    {
	if(GetIntValue(GetSubElement(First,"common"),"ID") != 0)
	{
	    LogError("First UI element is not a container (%d)!",GetIntValue(GetSubElement(First,"common"),"ID"));
	    return {};
	}


	LoadElementFromTree(&Container, First, Arena, Textures, TempArena, GlobalArchiveCollection, FontContainer);

	TAUIContainer * ContainerDetails = &Container.Container;
	ContainerDetails->NumberOfElements = CountElements(First)-1;
	ContainerDetails->Elements = PushArray(Arena, ContainerDetails->NumberOfElements, TAUIElement);
	Container.Textures = Textures;

	for(int i=0;i<ContainerDetails->NumberOfElements;i++)
	{
	    LoadElementFromTree(&ContainerDetails->Elements[i], GetNthElement(First, i+1), Arena, Textures, TempArena, GlobalArchiveCollection, FontContainer);
	    ContainerDetails->Elements[i].Textures = Textures;
	}
    }

  


    
    PopSubArena(TempArena, UIElementsArena);
    return Container;
}


void LoadCommonUITextures(GameState * CurrentGameState)
{
    SetupTextureContainer(&CurrentGameState->CommonGUITextures, COMMONUI_TEXTURE_WIDTH, COMMONUI_TEXTURE_HEIGHT, COMMONUI_MAX_TEXTURES, &CurrentGameState->GameArena);
    HPIEntry CommonUI = FindEntryInAllFiles("anims/commonGUI.GAF", &CurrentGameState->GlobalArchiveCollection, &CurrentGameState->TempArena);
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
	LoadAllTexturesFromHPIEntry(&CommonUI, &CurrentGameState->CommonGUITextures, &CurrentGameState->TempArena, CurrentGameState->PaletteData);
    }

}

void RenderTAUIElement(TAUIElement * Element, Texture2DShaderDetails * ShaderDetails, TextureContainer * ButtonFont, TextureContainer * CommonUIElements, TAUIContainer * Container = 0)
{
    switch(Element->ElementType)
    {
    case TAG_UI_CONTAINER:
    {
	Texture * Background =  Element->Container.Background;
	if(Background)
	{
	    DrawTexture2D(Element->Textures->Texture, Element->X, Element->Y, Element->Width, Element->Height, {{1,1,1}}, 1.0, ShaderDetails, Background->U, Background->V, Background->Widths[0], Background->Heights[0]);
	}
	for(int i=0;i<Element->Container.NumberOfElements;i++)
	{
	    RenderTAUIElement(&Element->Container.Elements[i], ShaderDetails, ButtonFont, CommonUIElements, &Element->Container);
	}
    }
    break;
    case TAG_BUTTON:
    {
	TAUIButton * Button = &Element->Button;
	Texture * ButtonBackground = GetTexture(Element->Name, Element->Textures);
	TextureContainer * ButtonTextureContainer = Element->Textures;
	int ButtonIndex =0;
	if(!ButtonBackground)
	{
	    ButtonBackground = GetTexture(Element->Name, CommonUIElements);
	    ButtonTextureContainer = CommonUIElements;
	    if(!ButtonBackground)
	    {
		ButtonBackground = GetTexture("Buttons0", CommonUIElements);
		float ElementWidthInUV = (float)Element->Width / CommonUIElements->TextureWidth;
		float ElementHeightInUV = (float)Element->Height / CommonUIElements->TextureHeight;
		float BestMatchAmount = (ButtonBackground->Widths[0] - ElementWidthInUV) + (ElementHeightInUV - ButtonBackground->Heights[0]);
		for(int i=1;i<ButtonBackground->NumberOfFrames;i++)
		{
		    float CurrentMatchAmount = (ButtonBackground->Widths[i] - ElementWidthInUV) + (ElementHeightInUV - ButtonBackground->Heights[i]);
		    if(fabs(CurrentMatchAmount) < fabs(BestMatchAmount) )
		    {
			BestMatchAmount = CurrentMatchAmount;
			ButtonIndex =i;
		    }
		}
	    }
	}
	Assert(ButtonBackground);
	float U = ButtonBackground->U;
	float V = ButtonBackground->V;
	for(int i=0;i<ButtonIndex;i++)
	{
	    U+=ButtonBackground->Widths[i];
	}
	DrawTexture2D(ButtonTextureContainer->Texture, Element->X, Element->Y, Element->Width, Element->Height, {{1,1,1}}, 1.0, ShaderDetails, U, V, ButtonBackground->Widths[ButtonIndex], ButtonBackground->Heights[ButtonIndex]);
	if(Button->Text)
	{
	    int Width = TextWidthInPixels(Button->Text, ButtonFont);
	    int Height = TextHeightInPixels(Button->Text,ButtonFont);
	    Draw2DFontText(Button->Text, Element->X+((Element->Width - Width)/2), Element->Y, ButtonFont, ShaderDetails,Height);
	}
    }
    break;
    case TAG_LABEL:
    {
	//TODO(Christof): update text for specially named labels here
	if(Element->Label.Text)
	{
	FNTFont * LabelFont = 0;
	for(int i=0;i<Container->NumberOfElements;i++)
	{
	    if(Container->Elements[i].ElementType == TAG_LABEL_FONT)
	    {
		LabelFont = Container->Elements[i].LabelFont.Font;
		break;
	    }
	}
	if(LabelFont)
	{
	}
	else
	{
	    int Width = TextWidthInPixels(Element->Label.Text, ButtonFont);
	    int Height = TextHeightInPixels(Element->Label.Text,ButtonFont);
	    Draw2DFontText(Element->Label.Text, Element->X+((Element->Width - Width)/2), Element->Y+((Element->Height - Height)/2), ButtonFont, ShaderDetails,Height);
	}
	}
    }
    break;
    }
}
