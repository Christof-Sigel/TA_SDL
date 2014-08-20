#version 400

in vec3 FPosition;
smooth in vec2 FTexCoord;
out vec4 out_Color;
uniform sampler2D UnitTexture;


void main(void)
{
    out_Color=texture2D(UnitTexture,FTexCoord);
    // out_Color=vec4(1,FTexCoord.x,FTexCoord.y,1);
}
