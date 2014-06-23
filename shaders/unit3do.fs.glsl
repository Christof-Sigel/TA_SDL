#version 400

in vec3 FPosition;
in vec2 FTexCoord;
out vec4 out_Color;



void main(void)
{
    out_Color=vec4(1,FTexCoord.x,FTexCoord.y,1);
}
