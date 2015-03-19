#version 400

in vec3 FPosition;
smooth in vec2 FTexCoord;
in vec3 FColor;
out vec4 out_Color;
uniform sampler2D UnitTexture;
uniform int RenderColor;
//uniform sampler1D ColorTexture;

void main(void)
{
    /*if(RenderColor==1)
    {
	out_Color=vec4(ColorPalette[FColorIndex],1);
    }
    else*/
    if(RenderColor==0)
    {
	out_Color=texture(UnitTexture,FTexCoord);
    }
    else
    {
	//out_Color=vec4(1,FTexCoord.x,FTexCoord.y,1);
	out_Color=vec4(FColor,1);
    }
}
