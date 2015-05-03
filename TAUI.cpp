enum FixedMenuItem
{
    FIXED_MENU_ITEM_NONE,
    FIXED_MENU_ITEM_SINGLE
    
};


struct TAUIElement
{
    char * Name;
    int ID, Association,X,Y,Width,Height,Attributes,ColorFore,ColorBack,TextureNumber,FontNumber;
    FixedMenuItem Item;
    
};


TAUIElement LoadGUIFromBuffer(char * Buffer)
{

    return{0};
}

