


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
	//TODO(Christof): Update UIElements to have background (will use this here)
    }
};


const int TNT_HEADER_ID=0x2000;
