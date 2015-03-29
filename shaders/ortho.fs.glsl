#version 400


noperspective in vec2 FTexCoord;
noperspective in vec2 FVertCoord;
out vec4 out_Color;
uniform sampler2D UnitTexture;
uniform vec3 uTextColor;



void main(void)
{
    vec2 FTexCoords=FTexCoord;//vec2(1,1)-FTexCoord;
    vec3 TextColor=uTextColor;
    TextColor=vec3(1.0,0.0,0);
    out_Color=texture(UnitTexture,FTexCoords);
    out_Color=vec4((vec3(1,1,1)-out_Color.xyz)*TextColor,out_Color.w);;
    //out_Color=vec4(FTexCoords.x,FTexCoords.y,1,1);
    //out_Color=vec4(FVertCoords.x,FVertCoords.y,1,1);
}
