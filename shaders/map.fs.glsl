#version 400

in vec2 FTexCoord;
out vec4 out_Color;
uniform sampler2D Texture;

void main(void)
{
    out_Color=texture(Texture,FTexCoord);
//    out_Color = vec4(1.0,0,0,1.0);
}
