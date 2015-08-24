#version 400

//in vec3 FPosition;
in vec4 FTexCoord;
in vec3 FColor;
out vec4 out_Color;
uniform sampler2D UnitTexture;

void main(void)
{
    /*if(RenderColor==1)
    {
	out_Color=vec4(ColorPalette[FColorIndex],1);
    }
    else*/
    out_Color=texture(UnitTexture,FTexCoord.xy/FTexCoord.w);
    if(FTexCoord.x<0)
    {

	//out_Color=vec4(1,FTexCoord.x,FTexCoord.y,1);
	out_Color=vec4(FColor,1);

//	out_Color=vec4(FTexCoord.x,0,1.0f,1);
    }
    //out_Color=vec4(0,0,1,1);
}
