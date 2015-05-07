

FontDetails Times32;
FontDetails Times24;
FontDetails Times16;

void LoadFonts()
{
    Times32=LoadFont("data/times.ttf",32);
    Times24=LoadFont("data/times.ttf",24);
    Times16=LoadFont("data/times.ttf",16);
}

void ReloadShaders();
TAMap TestMap={0};
std::vector<UnitDetails> Units;
void LoadCurrentModel();
int64_t StartTime=0;
int NumberOfFrames=0;
ShaderProgram UnitShader;
UIElement TestElement[5];
Object3d temp_model;
GLuint ProjectionMatrixLocation;
GLuint ModelMatrixLocation;
GLuint ViewMatrixLocation;

extern int ScreenWidth;
extern int ScreenHeight;

void Setup()
{
#ifdef __WINDOWS__
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerformaceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    LoadFonts();
    SetupUIElementRender();
    
    // ViewMatrix.Rotate(0,1,0, PI);
    //ViewMatrix.Rotate(0,1,0, -PI/4);
    //ViewMatrix.Move(1,0,0);
    
    ReloadShaders();

        
    LoadHPIFileCollection();
    LoadAllTextures();
    HPIEntry Map = FindEntryInAllFiles("maps/Greenhaven.tnt");
    if(Map.Name)
    {
	char * temp = (char*)malloc(Map.File.FileSize);
    
	if(LoadHPIFileEntryData(Map,temp))
	{
	    TestMap=LoadTNTFromBuffer(temp);
	}
	else
	    LogDebug("failed to load map buffer from hpi");
	free(temp);
    }
    else
	LogDebug("failed to load map");
    
	

    HPIEntry Entry=FindEntryInAllFiles("units");
    if(Entry.IsDirectory)
    {
	for(int i=0;i<Entry.Directory.NumberOfEntries;i++)
	{
	    char temp[Entry.Directory.Entries[i].File.FileSize];
	    if(LoadHPIFileEntryData(Entry.Directory.Entries[i],temp))
	    {
		UnitDetails deets;
		if(strstr(Entry.Directory.Entries[i].Name,".FBI"))
		{
		    LoadFBIFileFromBuffer(&deets,temp);
		    Units.push_back(deets);
		}
	    }
	}
    }
    UnloadCompositeEntry(&Entry);

    LoadCurrentModel();

    StartTime= GetTimeMillis();
}


void Teardown()
{
    int64_t EndTime=GetTimeMillis();
    LogDebug("%d frames in %.3fs, %.2f FPS",NumberOfFrames,(EndTime-StartTime)/1000.0,NumberOfFrames/((EndTime-StartTime)/1000.0));
    UnloadShaderProgram(UnitShader);
    UnloadHPIFileCollection();
    Unload3DO(&temp_model);
}


void ReloadShaders()
{
    if(UnitShader.ProgramID)
	UnloadShaderProgram(UnitShader);
    if(MapShader.ProgramID)
	UnloadShaderProgram(MapShader);
    UnitShader=LoadShaderProgram("shaders/unit3do.vs.glsl","shaders/unit3do.fs.glsl");
    
    glUseProgram(UnitShader.ProgramID);
    glUniform1i(GetUniformLocation(UnitShader,"UnitTexture"),0);

    MapShader=LoadShaderProgram("shaders/map.vs.glsl","shaders/map.fs.glsl");
    
    glUseProgram(MapShader.ProgramID);
    glUniform1i(GetUniformLocation(MapShader,"Texture"),0);

    ProjectionMatrixLocation = GetUniformLocation(UnitShader,"ProjectionMatrix");
    ModelMatrixLocation = GetUniformLocation(UnitShader,"ModelMatrix");
    ViewMatrixLocation = GetUniformLocation(UnitShader,"ViewMatrix");


    FontShader=LoadShaderProgram("shaders/font.vs.glsl","shaders/font.fs.glsl");
    glUseProgram(FontShader.ProgramID);
    glUniform1i(GetUniformLocation(FontShader,"Texture"),0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glUniform2iv(GetUniformLocation(FontShader,"Viewport"),1,viewport+2);
    FontPositionLocation=GetUniformLocation(FontShader,"Position");
    FontColorLocation=GetUniformLocation(FontShader,"TextColor");

    
    UIElementShaderProgram=LoadShaderProgram("shaders/UI.vs.glsl","shaders/UI.fs.glsl");
    glUseProgram(UIElementShaderProgram.ProgramID);

    UIElementPositionLocation = GetUniformLocation(UIElementShaderProgram,"Position");
    UIElementSizeLocation = GetUniformLocation(UIElementShaderProgram,"Size");
    UIElementColorLocation = GetUniformLocation(UIElementShaderProgram,"Color");
    UIElementBorderColorLocation = GetUniformLocation(UIElementShaderProgram,"BorderColor");
    UIElementBorderWidthLocation = GetUniformLocation(UIElementShaderProgram,"BorderWidth");
    UIElementAlphaLocation = GetUniformLocation(UIElementShaderProgram,"Alpha");

    glUniform2iv(GetUniformLocation(UIElementShaderProgram,"Viewport"),1,viewport+2);

}

void CheckResources()
{
    uint64_t UnitVertexShaderTime = GetFileModifiedTime("shaders/unit3do.vs.glsl");
    uint64_t UnitPixelShaderTime = GetFileModifiedTime("shaders/unit3do.fs.glsl");
    uint64_t MapVertexShaderTime = GetFileModifiedTime("shaders/map.vs.glsl");
    uint64_t MapPixelShaderTime = GetFileModifiedTime("shaders/map.fs.glsl");

    uint64_t UIVertexShaderTime = GetFileModifiedTime("shaders/UI.vs.glsl");
    uint64_t UIPixelShaderTime = GetFileModifiedTime("shaders/UI.fs.glsl");

    uint64_t TextVertexShaderTime = GetFileModifiedTime("shaders/font.vs.glsl");
    uint64_t TextPixelShaderTime = GetFileModifiedTime("shaders/font.fs.glsl");
    if(UnitVertexShaderTime > UnitShader.VertexFileModifiedTime || UnitPixelShaderTime > UnitShader.PixelFileModifiedTime
       || MapPixelShaderTime > MapShader.VertexFileModifiedTime || MapVertexShaderTime > MapShader.PixelFileModifiedTime
       || UIVertexShaderTime > UIElementShaderProgram.VertexFileModifiedTime || UIPixelShaderTime > UIElementShaderProgram.PixelFileModifiedTime
       || TextVertexShaderTime > FontShader.VertexFileModifiedTime || TextPixelShaderTime > FontShader.PixelFileModifiedTime
	)
    ReloadShaders();
}
    
