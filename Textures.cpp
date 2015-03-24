
//NOTE: Textures in "textures"

void LoadTextures(HPIFile* HPI)
{
    HPIEntry Textures = FindHPIEntry(HPI,"textures");
    if(!Textures.IsDirectory)
    {
	LogError("Found a file instead of a directory while trying to load textures from %s",HPI->Name);
	return;
    }
    LogDebug("Loading %d Textures From %s",Textures.Directory.NumberOfEntries,HPI->Name);
}
