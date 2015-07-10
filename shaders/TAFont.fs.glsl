#version 400

out vec4 out_Color;

uniform vec3 Color;
uniform float Alpha;
uniform sampler2D Texture;


noperspective in vec2 FTexCoord;


void main(void)
{
    out_Color=texture(Texture,FTexCoord);
    out_Color=vec4((vec3(1,1,1)-out_Color.xyz)*Color,out_Color.w);
//    out_Color=vec4(FTexCoord.x, FTexCoord.y, 0,1);
}
    
