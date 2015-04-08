#version 400


noperspective in vec2 FTexCoord;
out vec4 out_Color;
uniform sampler2D Texture;
uniform vec3 TextColor;



void main(void)
{
    vec2 FTexCoords=FTexCoord;//vec2(1,1)-FTexCoord;
    out_Color=texture(Texture,FTexCoords);
    out_Color=vec4((vec3(1,1,1)-out_Color.xyz)*TextColor,out_Color.w);;
    //out_Color=vec4(FTexCoords.x,FTexCoords.y,1,1);
    //out_Color=vec4(FVertCoords.x,FVertCoords.y,1,1);
//    out_Color=vec4(TextColor,1.0);
}
