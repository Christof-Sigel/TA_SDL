internal char * GetStringValue(FILE_UIElement * Element, const char * Name)
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

internal int GetIntValue(FILE_UIElement * Element, const char * Name)
{
    char * Value=GetStringValue(Element,Name);
    if(Value)
	return atoi(Value);
    return -1;
}

internal float GetFloat(FILE_UIElement * Element, const char * Name)
{
    char * Value= GetStringValue(Element,Name);
    if(Value)
	return (float)atof(Value);
    return -1;
}

internal FILE_UIElement * GetSubElement(FILE_UIElement * Root, const char * Name)
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

internal FILE_UIElement * GetNthElement(FILE_UIElement * Start, int N)
{
    if(N ==0 || !Start)
	return Start;
    return GetNthElement(Start->Next, N-1);
}

internal int CountElements(FILE_UIElement * First)
{
    int count = 0;
    while(First)
    {
	First = First->Next;
	count++;
    }
    return count;
}

internal FILE_UINameValue * LoadUINameValueFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
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

internal FILE_UIElement * LoadUIElementFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
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

internal FILE_UIElement * LoadUIElementsFromBuffer(char ** InBuffer, char * End, MemoryArena * Arena)
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

internal void LoadElementFromTree(TAUIElement * Element, FILE_UIElement * Tree, MemoryArena * Arena, TextureContainer * Textures, MemoryArena * TempArena, HPIFileCollection * GlobalArchiveCollection, FontContainer * FontContainer, const char * FileName)
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
    size_t name_len = strlen(Name);
    Element->Name = PushArray(Arena, name_len+1, char);
    if(Element->Name)
    {
	memcpy(Element->Name, Name, name_len);
	Element->Name[name_len]=0;
    }

    char * Help = GetStringValue(Common, "help");
    if(Help)
    {
	size_t help_len = strlen(Help);
	Element->Help = PushArray(Arena, help_len+1, char);
	if(Element->Help)
	{
	    memcpy(Element->Help, Help, help_len);
	    Element->Help[help_len]=0;
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
	else if(CaseInsensitiveMatch(FileName, "Single.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/SingleBG.pcx", GlobalArchiveCollection, TempArena);
	}
	else if(CaseInsensitiveMatch(FileName, "NewGame.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/playanygame4.pcx", GlobalArchiveCollection, TempArena);
	}
	else if(CaseInsensitiveMatch(Name, "Skirmish.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/skirmsetup4x.pcx", GlobalArchiveCollection, TempArena);
	}
	else if(CaseInsensitiveMatch(FileName, "LoadGame.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/Dloadgame2.pcx", GlobalArchiveCollection, TempArena);
	}
	else if(CaseInsensitiveMatch(FileName, "startopt.gui"))
	{
	    Element->Container.Background = AddPCXToTextureContainer(Textures,"bitmaps/Options4x.pcx", GlobalArchiveCollection, TempArena);
	}
	else
	{
	    Element->Container.Background = GetTexture(Name,Textures);
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
	    size_t text_len = strlen(Text);
	    Element->Button.Text = PushArray(Arena, text_len+1, char);
	    if(Element->Button.Text)
	    {
		memcpy(Element->Button.Text, Text, text_len);
		Element->Button.Text[text_len]=0;
	    }
	}
	Element->Button.Disabled = b8(GetIntValue(Tree, "grayedout"));
    }
    break;
    case TAG_TEXTFIELD:
    {
	Element->TextBox.MaximumCharacters = GetIntValue(Tree, "maxchars");
    }
    break;
    case TAG_SCROLLBAR:
    {
	/*Element->ScrollBar.Maximum = GetIntValue(Tree, "range");
	Element->ScrollBar.Position = GetIntValue(Tree, "knobpos");
	Element->ScrollBar.KnobSize = GetIntValue(Tree, "knobsize");*/
	//NOTE(Christof): Any slider we care about hase these values set from stuff its connected to anyway
    }
    break;
    case TAG_LABEL:
    {
	//TODO(Christof): Load Linked button
	char * Text = GetStringValue(Tree, "text");
	if(Text)
	{
	    size_t len = strlen(Text);
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
	Element->DynamicImage.DisplaySelectionRectangle = (b8)GetIntValue(Tree, "hotornot");
    }
    break;
    case TAG_LABEL_FONT:
    {
	char * FontFileName = GetStringValue(Tree, "filename");
	Element->LabelFont.Font=GetFont(FontContainer,FontFileName, GlobalArchiveCollection, TempArena);
    }
    break;
    case TAG_LISTBOX:
    case TAG_IMAGE:
	//NOTE(Christof): no extra loading required for these
	break;
    }
    if(CaseInsensitiveMatch("single",Name))
    {
	Element->ElementName = ELEMENT_NAME_SINGLEPLAYER;
    }
    else if(CaseInsensitiveMatch("multi",Name))
    {
	Element->ElementName = ELEMENT_NAME_MULTIPLAYER;
    }
    else if(CaseInsensitiveMatch("debugstring",Name))
    {
	Element->ElementName = ELEMENT_NAME_DEBUGSTRING;
    }
    else if(CaseInsensitiveMatch("Exit",Name))
    {
	Element->ElementName = ELEMENT_NAME_EXIT;
    }
    else if(CaseInsensitiveMatch("NewCamp",Name))
    {
	Element->ElementName = ELEMENT_NAME_NEW_CAMPAIGN;
    }
    else if(CaseInsensitiveMatch("LoadGame",Name))
    {
	Element->ElementName = ELEMENT_NAME_LOAD_GAME;
    }
    else if(CaseInsensitiveMatch("Skirmish",Name))
    {
	Element->ElementName = ELEMENT_NAME_SKIRMISH;
    }
    else if(CaseInsensitiveMatch("PrevMenu",Name)||CaseInsensitiveMatch("Prev",Name))
    {
	Element->ElementName = ELEMENT_NAME_PREVIOUS_MENU;
    }
    else if(CaseInsensitiveMatch("Options",Name))
    {
	Element->ElementName = ELEMENT_NAME_OPTIONS;
    }
    else if(CaseInsensitiveMatch("OK",Name))
    {
	Element->ElementName = ELEMENT_NAME_OK;
    }
    else if(CaseInsensitiveMatch("LOAD",Name))
    {
	Element->ElementName = ELEMENT_NAME_LOAD;
    }
    else if(CaseInsensitiveMatch("CANCEL",Name))
    {
	Element->ElementName = ELEMENT_NAME_CANCEL;
    }
    else if(CaseInsensitiveMatch("INTRO",Name))
    {
	Element->ElementName = ELEMENT_NAME_INTRO;
    }
    else if(CaseInsensitiveMatch("Credits",Name))
    {
	Element->ElementName = ELEMENT_NAME_CREDITS;
    }
    else if(CaseInsensitiveMatch("Start",Name))
    {
	Element->ElementName = ELEMENT_NAME_START;
    }
    else if(CaseInsensitiveMatch("Campaign",Name))
    {
	Element->ElementName = ELEMENT_NAME_CAMPAIGN;
    }
    else if(CaseInsensitiveMatch("CampaignKnob",Name))
    {
	Element->ElementName = ELEMENT_NAME_CAMPAIGN_KNOB;
    }
    else if(CaseInsensitiveMatch("MissionsKnob",Name))
    {
	Element->ElementName = ELEMENT_NAME_MISSIONS_KNOB;
    }
    else if(CaseInsensitiveMatch("Missions",Name))
    {
	Element->ElementName = ELEMENT_NAME_MISSIONS;
    }
    else if(CaseInsensitiveMatch("Side0",Name))
    {
	Element->ElementName = ELEMENT_NAME_SIDE_0;
    }
    else if(CaseInsensitiveMatch("Side1",Name))
    {
	Element->ElementName = ELEMENT_NAME_SIDE_1;
    }
    else if(CaseInsensitiveMatch("SIDENAME",Name))
    {
	Element->ElementName = ELEMENT_NAME_SIDE_NAME;
    }
    else if(CaseInsensitiveMatch("Difficulty",Name))
    {
	Element->ElementName = ELEMENT_NAME_DIFFICULTY;
    }
    else if(CaseInsensitiveMatch("Arm",Name))
    {
	Element->ElementName = ELEMENT_NAME_ARM;
    }
    else if(CaseInsensitiveMatch("CORE",Name))
    {
	Element->ElementName = ELEMENT_NAME_CORE;
    }
    else if(CaseInsensitiveMatch("GAMES",Name))
    {
	Element->ElementName = ELEMENT_NAME_GAMES;
    }
    else if(CaseInsensitiveMatch("SLIDER",Name))
    {
	Element->ElementName = ELEMENT_NAME_SLIDER;
    }
    else if(CaseInsensitiveMatch("GAMENAME",Name))
    {
	Element->ElementName = ELEMENT_NAME_GAME_NAME;
    }
    else if(CaseInsensitiveMatch("GAMETYPE",Name))
    {
	Element->ElementName = ELEMENT_NAME_GAME_TYPE;
    }
    else if(CaseInsensitiveMatch("MISSION",Name))
    {
	Element->ElementName = ELEMENT_NAME_MISSION;
    }
    else if(CaseInsensitiveMatch("TIME",Name))
    {
	Element->ElementName = ELEMENT_NAME_TIME;
    }
    else if(CaseInsensitiveMatch("SIDE",Name))
    {
	Element->ElementName = ELEMENT_NAME_SIDE;
    }
    else if(CaseInsensitiveMatch("RADAR",Name))
    {
	Element->ElementName = ELEMENT_NAME_RADAR;
    }
    else if(CaseInsensitiveMatch("DIFF",Name))
    {
	Element->ElementName = ELEMENT_NAME_DIFF;
    }
    else if(CaseInsensitiveMatch("DELETE",Name))
    {
	Element->ElementName = ELEMENT_NAME_DELETE;
    }
    else if(CaseInsensitiveMatch("SaveGame",Name))
    {
	Element->ElementName = ELEMENT_NAME_SAVE;
    }
    else if(CaseInsensitiveMatch("Skirmish.gui",Name))
    {
	Element->ElementName = ELEMENT_NAME_SKIRMISH_GUI;
    }
    else if(CaseInsensitiveMatch("HEADER",Name))
    {
	Element->ElementName = ELEMENT_NAME_HEADER;
    }
    else if(CaseInsensitiveMatch("TEXT",Name))
    {
	Element->ElementName = ELEMENT_NAME_TEXT;
    }
    else if(CaseInsensitiveMatch("SelectMap",Name))
    {
	Element->ElementName = ELEMENT_NAME_SELECT_MAP;
    }
    else if(CaseInsensitiveMatch("StartLocation",Name))
    {
	Element->ElementName = ELEMENT_NAME_START_LOCATION;
    }
    else if(CaseInsensitiveMatch("CommanderDeath",Name))
    {
	Element->ElementName = ELEMENT_NAME_COMMANDER_DEATH;
    }
    else if(CaseInsensitiveMatch("Mapping",Name))
    {
	Element->ElementName = ELEMENT_NAME_MAPPING;
    }
    else if(CaseInsensitiveMatch("MapName",Name))
    {
	Element->ElementName = ELEMENT_NAME_MAP_NAME;
    }
    else if(CaseInsensitiveMatch("LineOfSight",Name))
    {
	Element->ElementName = ELEMENT_NAME_LINE_OF_SIGHT;
    }
    else if(CaseInsensitiveMatch("HELPTEXT",Name))
    {
	Element->ElementName = ELEMENT_NAME_HELP_TEXT;
    }
    else if(CaseInsensitiveMatch("SOUND",Name))
    {
	Element->ElementName = ELEMENT_NAME_SOUND;
    }
    else if(CaseInsensitiveMatch("SPEEDS",Name))
    {
	Element->ElementName = ELEMENT_NAME_SPEEDS;
    }
    else if(CaseInsensitiveMatch("VISUALS",Name))
    {
	Element->ElementName = ELEMENT_NAME_VISUALS;
    }
    else if(CaseInsensitiveMatch("MUSIC",Name))
    {
	Element->ElementName = ELEMENT_NAME_MUSIC;
    }
    else if(CaseInsensitiveMatch("RESTORE",Name))
    {
	Element->ElementName = ELEMENT_NAME_RESTORE;
    }
    else if(CaseInsensitiveMatch("UNDO",Name))
    {
	Element->ElementName = ELEMENT_NAME_UNDO;
    }
    else if(CaseInsensitiveMatch("MainMenu.gui",Name))
    {
	Element->ElementName = ELEMENT_NAME_MAIN_MENU_GUI;
    }
    else if(CaseInsensitiveMatch("AnyMsn",Name))
    {
	Element->ElementName = ELEMENT_NAME_ANY_MISSION;
    }

    else
    {
	LogDebug("Unknown Element: %s",Name);
    }


    if(Element->ElementName == ELEMENT_NAME_SIDE_0 || Element->ElementName == ELEMENT_NAME_SIDE_1)
    {
	Element->Width = 158;
	Element->Height = 158;
    }

    if(Element->ElementName == ELEMENT_NAME_DELETE)
    {
	Element->Visible = 0;
    }


    //TODO(Christof): make this actually check for campaigns before enabling?
    if(Element->ElementName == ELEMENT_NAME_CAMPAIGN_KNOB
       || Element->ElementName == ELEMENT_NAME_MISSIONS
       || Element->ElementName == ELEMENT_NAME_MISSIONS_KNOB
       ||Element->ElementName == ELEMENT_NAME_CAMPAIGN)
    {
	Element->Visible = 1;
    }
    if(Element->ElementName == ELEMENT_NAME_CAMPAIGN_KNOB)
    {
	Element->Height = 46;
	Element->Y = 308;
    }
    if( Element->ElementName == ELEMENT_NAME_MISSIONS)
    {
	Element->Height = 64;
	Element->Width = 318;
    }
    if( Element->ElementName == ELEMENT_NAME_MISSIONS_KNOB)
    {
	Element->Height = 64;
    }
    if(Element->ElementName == ELEMENT_NAME_CAMPAIGN)
    {
	Element->Height = 46;
	Element->Width = 318;
	Element->Y = 308;
    }
}

internal TAUIElement LoadGUIFromBuffer(char * Buffer, char * End, MemoryArena * Arena, MemoryArena * TempArena, const char * FileName, HPIFileCollection * GlobalArchiveCollection, u8 * PaletteData, FontContainer * FontContainer)
{
    TextureContainer * Textures =PushStruct(Arena, TextureContainer);
    //TODO(Christof): Better texture memory allocation needed, perhaps on Temp and only full size when initially loading gafs?
    SetupTextureContainer(Textures, 1024,1024, 40, Arena);

    const s32 MAX_STRING = 64;
    char GafFileName[MAX_STRING];
    snprintf(GafFileName,MAX_STRING,"anims/%s",FileName);
    size_t len = strlen(GafFileName);
    GafFileName[len-3]='g';
    GafFileName[len-2]='a';
    GafFileName[len-1]='f';

    HPIEntry UITextures = FindEntryInAllFiles(GafFileName, GlobalArchiveCollection, TempArena);
    if(UITextures.IsDirectory)
    {
	LogError("Unexpectedly found a directory while trying to load %s", GafFileName);
    }
    else if(!UITextures.Name)
    {
	//NOTE(Christof): just silently fail here, most guis don't have a .gaf
	glGenTextures(1,&Textures->Texture);
	glBindTexture(GL_TEXTURE_2D,Textures->Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	u8* TextureData = PushArray(TempArena, Textures->TextureWidth * Textures->TextureHeight *4, u8);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,Textures->TextureWidth,Textures->TextureHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData);
	PopArray(TempArena, TextureData, Textures->TextureWidth * Textures->TextureHeight *4, u8);
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
	LoadElementFromTree(&Container, First, Arena, Textures, TempArena, GlobalArchiveCollection, FontContainer,FileName );

	TAUIContainer * ContainerDetails = &Container.Container;
	ContainerDetails->NumberOfElements = CountElements(First)-1;
	ContainerDetails->Elements = PushArray(Arena, ContainerDetails->NumberOfElements, TAUIElement);
	Container.Textures = Textures;

	for(int i=0;i<ContainerDetails->NumberOfElements;i++)
	{
	    LoadElementFromTree(&ContainerDetails->Elements[i], GetNthElement(First, i+1), Arena, Textures, TempArena, GlobalArchiveCollection, FontContainer,0);
	    ContainerDetails->Elements[i].Textures = Textures;
	}
    }
    PopSubArena(TempArena, UIElementsArena);
    return Container;
}

internal TAUIElement LoadGUI(const char * FileName, HPIFileCollection * GlobalArchiveCollection, MemoryArena * TempArena, MemoryArena * GameArena , u8* PaletteData, FontContainer * FontContainer )
{
    TAUIElement Result={};
    const int MAX_STRING = 128;
    char FullFileName[MAX_STRING];
    snprintf(FullFileName, MAX_STRING, "guis/%s", FileName);
    HPIEntry GUIFile = FindEntryInAllFiles(FullFileName,GlobalArchiveCollection, TempArena);
    if(!GUIFile.IsDirectory)
    {
	u8 * GUIBuffer = PushArray(TempArena, GUIFile.File.FileSize,u8 );
	LoadHPIFileEntryData(GUIFile,GUIBuffer,TempArena);
	Result = LoadGUIFromBuffer((char*)GUIBuffer, (char*)GUIBuffer + GUIFile.File.FileSize, GameArena, TempArena, FileName, GlobalArchiveCollection, PaletteData, FontContainer);
	PopArray(TempArena, GUIBuffer,  GUIFile.File.FileSize, u8 );
    }
    if(!Result.Name)
    {
	LogError("Failed to load %s", FileName);
    }
    return Result;
}

internal void LoadCommonUITextures(GameState * CurrentGameState)
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

internal void RenderTAUIElement(TAUIElement * Element, s32 XOffset, s32 YOffset, Texture2DShaderDetails * ShaderDetails, TextureContainer * Font11, TextureContainer *Font12, TextureContainer * CommonUIElements, DebugRectShaderDetails * DebugRectDetails, TAUIElement * Container = 0)
{
    if(!Element->Visible)
	return;

    s32 X= Element->X + XOffset;
    s32 Y = Element->Y + YOffset;
    r32 Width = (r32)Element->Width;
    r32 Height = (r32)Element->Height;
    switch(Element->ElementType)
    {
    case TAG_UI_CONTAINER:
    {
	Texture * Background =  Element->Container.Background;
	if(Background)
	{
	    DrawTexture2D(Element->Textures->Texture, float(X), float(Y), float(Element->Width), float(Element->Height), {{1,1,1}}, 1.0, ShaderDetails, Background->U, Background->V, Background->Width, Background->Height);
	}
	for(int i=0;i<Element->Container.NumberOfElements;i++)
	{
	    RenderTAUIElement(&Element->Container.Elements[i], X, Y, ShaderDetails, Font11, Font12, CommonUIElements, DebugRectDetails, Element);
	}
    }
    break;
    case TAG_BUTTON:
    {
	TAUIButton * Button = &Element->Button;
	Texture * ButtonBackground = GetTexture(Element->Name, Element->Textures);
	TextureContainer * ButtonTextureContainer = Element->Textures;

	int ButtonOffset = Button->Pressed;
	if(!ButtonBackground)
	{
	    ButtonBackground = GetTexture(Element->Name,  CommonUIElements);
	    ButtonTextureContainer = CommonUIElements;
	    if(!ButtonBackground)
	    {
		if(Button->Stages > 0)
		{
		    Assert(Button->Stages <4);
		    const s32 MAX_STRING = 20;
		    char ButtonName[MAX_STRING];
		    snprintf(ButtonName,MAX_STRING,"stagebuttn%d", Button->Stages);
		    ButtonBackground = GetTexture(ButtonName, CommonUIElements);
		}
		else
		{
		    ButtonBackground = GetTexture("Buttons0", CommonUIElements);
		    float ElementWidthInUV = (float)Element->Width / CommonUIElements->TextureWidth;
		    float ElementHeightInUV = (float)Element->Height / CommonUIElements->TextureHeight;
		    float BestMatchAmount = (ButtonBackground->Width - ElementWidthInUV) + (ElementHeightInUV - ButtonBackground->Height);
		    Texture * BestMatch = ButtonBackground;
		    for(int i=4;i<ButtonBackground->NumberOfFrames;i+=4)
		    {
			ButtonBackground = GetTexture(ButtonBackground, i);
			float CurrentMatchAmount = (ButtonBackground->Width - ElementWidthInUV) + (ElementHeightInUV - ButtonBackground->Height);
			if(fabs(CurrentMatchAmount) < fabs(BestMatchAmount) )
			{
			    BestMatchAmount = CurrentMatchAmount;
			    BestMatch = ButtonBackground;
			}
		    }
		    ButtonBackground = BestMatch;
		}
		Width = ButtonBackground->Width * CommonUIElements->TextureWidth;
		Height = ButtonBackground->Height * CommonUIElements->TextureHeight;
		if(Button->Pressed && Button->Stages >0)
		{
		    ButtonOffset = Button->Stages + 1; // == Stages is nothing selected
		}
		else if (Button->Stages >0)
		{
		    ButtonOffset = Button->CurrentStage;
		}

	    }
	}
	if(Element->Button.Disabled)
	{
	    ButtonOffset = 3;
	}
	ButtonBackground = GetTexture(ButtonBackground, ButtonBackground->FrameNumber + ButtonOffset);
	float U = ButtonBackground->U;
	float V = ButtonBackground->V;
	DrawTexture2D(ButtonTextureContainer->Texture, float(X), float(Y), Width, Height, {{1,1,1}}, 1.0, ShaderDetails, U, V, ButtonBackground->Width, ButtonBackground->Height);

//	DrawDebugRect(DebugRectDetails, X , Y, Width, Height, {1,1,1} , 2.0f );
	if(Button->Text)
	{
	    const s32 MAX_STRING = 64;
	    char Text[MAX_STRING];
	    snprintf(Text, MAX_STRING, "%s", Button->Text);
	    char * ButtonText = Text;
	    if(Button->Stages > 0)
	    {
		char * c = Text;
		int i=0;
		while(*++c)
		{
		    if(*c == '|')
		    {
			*c=0;
			i++;
			if(i==Button->CurrentStage)
			    ButtonText = c+1;
		    }
		}
		X += 5;
	    }
	    else
	    {
		FontDimensions TD = TextSizeInPixels(ButtonText, Font12);
		X = X + (s32)((Width - TD.Width)/2);
	    }
	    Y = Y + (s32)((Height - 18)/2); //NOTE(Christof): 18 is the pixel height of Font12

	    DrawTextureFontText(ButtonText,X,Y , Font12, ShaderDetails);
	}

    }
    break;
    case TAG_LABEL:
    {
	//TODO(Christof): update text for specially named labels here
	if(Element->Label.Text)
	{
	    FNTFont * LabelFont = 0;
	    for(int i=0;i<Container->Container.NumberOfElements;i++)
	    {
		if(Container->Container.Elements[i].ElementType == TAG_LABEL_FONT)
		{
		    LabelFont = Container->Container.Elements[i].LabelFont.Font;
		    break;
		}
	    }
	    if(LabelFont)
	    {
		//TODO(Christof): FNT font label rendering here
	    }
	    else
	    {
		DrawTextureFontText(Element->Label.Text,X, Y, Font11, ShaderDetails);
	    }
	}
    }
    break;
    case TAG_LISTBOX:
	for(s32 i=0 ; i<Element->ListBox.NumberOfDisplayableItems ; i++)
	{
	    s32 index = Element->ListBox.DisplayItemIndex + i;
	    if(index == Element->ListBox.SelectedIndex)
	    {
		DrawDebugRect(DebugRectDetails, (r32)X , (r32)(Y + i * LIST_ITEM_HEIGHT), Width, LIST_ITEM_HEIGHT, {1,1,1} , 0.0f , 1.0f, {0,0.25,0}, 0.5f);
	    }
	    DrawTextureFontText(Element->ListBox.ItemStrings[index], X+2, Y + (i*LIST_ITEM_HEIGHT), Font12, ShaderDetails, 1.0,{2.0,2.0,2.0});
	}
	break;

    case TAG_SCROLLBAR:
	{
	    if(!Element->ScrollBar.ListBox)
	    {
		for(int i=0;i<Container->Container.NumberOfElements;i++)
		{
		    if(Container->Container.Elements[i].Association == Element->Association && Container->Container.Elements[i].ElementType == TAG_LISTBOX )
		    {
			Element->ScrollBar.ListBox = &Container->Container.Elements[i].ListBox;
			break;
		    }
		}
	    }

	     s32 NumberOfDisplayableItems = Element->ScrollBar.ListBox->NumberOfDisplayableItems;
	    s32 NumberOfItems = Element->ScrollBar.ListBox->NumberOfItems;
	    s32 DisplayIndex = Element->ScrollBar.ListBox->DisplayItemIndex;


	    if(NumberOfItems > NumberOfDisplayableItems)
	    {
	    s32 OY=Y;
	    //TODO(Christof): Check for and render horizontal sliders (only ones we care about at the moment and vertical)
	    Texture * Slider = GetTexture("SLIDERS", CommonUIElements);
	    //Draw  arrow
	    Slider = GetTexture(Slider, 6 + (Element->ScrollBar.Pressed ==1));
	    s32 TotalHeight =0;
	    s32 SliderHeight = (s32)(Slider->Height * CommonUIElements->TextureHeight);
	    DrawTexture2D(CommonUIElements->Texture, float(X), float(Y), Width, (r32)SliderHeight, {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    Y += SliderHeight;
	    TotalHeight += SliderHeight;
	    s32 TopY = Y;

	    //Draw top
	    Slider = GetTexture(Slider, 0);
	    SliderHeight = (s32)(Slider->Height * CommonUIElements->TextureHeight);
	    DrawTexture2D(CommonUIElements->Texture, float(X), float(Y), Width, (r32)SliderHeight, {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    Y += SliderHeight;
	    TotalHeight += SliderHeight;
	    s32 MiddleY = Y;

	    //Draw Bottom arrow
	    Slider = GetTexture(Slider, 8 + (Element->ScrollBar.Pressed ==2));
	    SliderHeight = (s32)(Slider->Height * CommonUIElements->TextureHeight);
	    Y = OY + Element->Height - SliderHeight;
	    DrawTexture2D(CommonUIElements->Texture, float(X), float(Y), Width, (r32)SliderHeight, {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    TotalHeight += SliderHeight;
	    s32 BottomY = Y;

	    //Draw Bottom
	    Slider = GetTexture(Slider, 2);
	    SliderHeight = (s32)(Slider->Height * CommonUIElements->TextureHeight);
	    Y -= SliderHeight;


	    DrawTexture2D(CommonUIElements->Texture, float(X), float(Y), Width, (r32)SliderHeight, {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    TotalHeight += SliderHeight;

	    s32 MiddleHeight = Element->Height - TotalHeight;
	    if(MiddleHeight > 0)
	    {
		Slider = GetTexture(Slider, 1);
		DrawTexture2D(CommonUIElements->Texture, float(X), float(MiddleY), Width, (r32)MiddleHeight, {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    }



	    TopY+=4;
	    BottomY-=4;
		if(DisplayIndex > NumberOfItems - NumberOfDisplayableItems)
		    DisplayIndex = NumberOfItems - NumberOfDisplayableItems;
		s32 InnerSize = BottomY - TopY;
		r32 KnobProportion = (r32) NumberOfDisplayableItems / NumberOfItems ;
		Element->ScrollBar.KnobSize = InnerSize * KnobProportion;
		if(Element->ScrollBar.KnobSize < 10)
		    Element->ScrollBar.KnobSize = 10;
		Element->ScrollBar.KnobPosition = ((r32) DisplayIndex / (NumberOfItems - NumberOfDisplayableItems)) * (InnerSize - Element->ScrollBar.KnobSize);

		//Draw Top of knob
		Slider = GetTexture(Slider, 3);
		DrawTexture2D(CommonUIElements->Texture, float(X), float(TopY+Element->ScrollBar.KnobPosition), Width, 1 , {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);

		//Draw Middle of knob
		Slider = GetTexture(Slider, 4);
		DrawTexture2D(CommonUIElements->Texture, float(X), float(TopY+Element->ScrollBar.KnobPosition+1), Width, Element->ScrollBar.KnobSize-2 , {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);

		//Draw bottom of knob
		Slider = GetTexture(Slider, 5);
		DrawTexture2D(CommonUIElements->Texture, float(X), float(TopY+Element->ScrollBar.KnobPosition+Element->ScrollBar.KnobSize-1), Width, 1 , {{1,1,1}}, 1.0, ShaderDetails, Slider->U, Slider->V, Slider->Width, Slider->Height);
	    }
	}

	break;
    }
}

internal TAUIElement * ProcessMouse(TAUIElement * Root, s32 MouseX, s32 MouseY, b32 Down, b32 WasDown, s32 XOffset, s32 YOffset)
{
    Assert(Root->ElementType == TAG_UI_CONTAINER);
    TAUIElement * Result = 0;
    b32 Clicked = !Down && WasDown;
    XOffset += Root->X;
    YOffset += Root->Y;

    for(s32 i=0;i<Root->Container.NumberOfElements;i++)
    {
	TAUIElement * Element = &Root->Container.Elements[i];
	s32 X = XOffset + Element->X, Y = YOffset + Element->Y;
	if(Element->Visible && MouseX < X + Element->Width && MouseX > X
	   && MouseY < Y + Element->Height && MouseY > Y)
	{
	    if(Clicked && ( (Element->ElementType != TAG_BUTTON && Element->ElementType != TAG_SCROLLBAR) || (!Element->Button.Disabled && Element->Button.Pressed)))
	    {
		Result = Element;
	    }
	    switch(Element->ElementType)
	    {
	    case TAG_BUTTON:
		if(Element->ElementName == ELEMENT_NAME_SIDE_0||
		   Element->ElementName == ELEMENT_NAME_SIDE_1||
		   Element->ElementName == ELEMENT_NAME_ARM ||
		   Element->ElementName == ELEMENT_NAME_CORE)
		{
		}
		else
		{
		    if(Element->Button.Stages > 0 && Clicked)
		    {
			Element->Button.CurrentStage++;
			if(Element->Button.CurrentStage >= Element->Button.Stages)
			    Element->Button.CurrentStage = 0;
		    }
		    Element->Button.Pressed = Down &&(!WasDown || Element->Button.Pressed);
		}
		break;
	    case TAG_SCROLLBAR:
		if(Down)
		{
		    if(MouseY < Y+10)
		    {
			if(!WasDown)
			    Element->ScrollBar.Pressed = 1;
			if(Clicked && Element->ScrollBar.Pressed == 1)
			{
			    if(Element->ScrollBar.ListBox->DisplayItemIndex >0)
			    {
				Element->ScrollBar.ListBox->DisplayItemIndex--;
			    }
			}
		    }
		    else if(MouseY > Y + Element->Height -10)
		    {
			if(!WasDown)
			    Element->ScrollBar.Pressed = 2;
			if(Clicked && Element->ScrollBar.Pressed==2)
			{
			    if(Element->ScrollBar.ListBox->DisplayItemIndex < Element->ScrollBar.ListBox->NumberOfItems - Element->ScrollBar.ListBox->NumberOfDisplayableItems)
			    {
				Element->ScrollBar.ListBox->DisplayItemIndex++;
			    }
			}
		    }
		}
		break;
	    case TAG_LISTBOX:
		if(Clicked)
		{
		    Element->ListBox.SelectedIndex = (MouseY - Y)/LIST_ITEM_HEIGHT + Element->ListBox.DisplayItemIndex;
		}
		break;
	    }

	}
	else
	{
	    switch(Element->ElementType)
	    {
	    case TAG_BUTTON:
		if(Element->ElementName == ELEMENT_NAME_SIDE_0||
		   Element->ElementName == ELEMENT_NAME_SIDE_1||
		   Element->ElementName == ELEMENT_NAME_ARM ||
		   Element->ElementName == ELEMENT_NAME_CORE)
		{
		}
		else
		{
		    if(Element->Button.Pressed)
		    {
			Element->Button.Pressed = Down;
		    }
		}
		break;
	    case TAG_SCROLLBAR:
		if(Element->ScrollBar.Pressed && !Down)
		{
		    Element->ScrollBar.Pressed = 0;
		}

		break;
	    }
	}
    }
    return Result;
}

internal TAUIElement * GetElementByName(TAUIElementName Name, TAUIElement * Root)
{
    Assert(Root->ElementType == TAG_UI_CONTAINER);
    for(int i=0;i<Root->Container.NumberOfElements;i++)
    {
	if(Root->Container.Elements[i].ElementName == Name)
	{
	    return &Root->Container.Elements[i];
	}
    }
    return 0;
}
