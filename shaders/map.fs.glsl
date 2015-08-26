#version 400

in vec2 FTexCoord;
out vec4 out_Color;
uniform sampler2D Texture;
uniform float SeaLevel;
in vec3 FPos;

void main(void)
{
    out_Color=texture(Texture,FTexCoord);
    if(FPos.y <= SeaLevel)
    {
	out_Color = mix(out_Color,vec4(1.0,0,1.0,1.0),0.5);
    }
}
