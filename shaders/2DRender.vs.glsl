#version 400

layout(location=0) in vec2 VPosition;
layout(location=1) in vec2 VTexCoord;


noperspective out vec2 FTexCoord;

uniform ivec2 Viewport;
uniform vec2 Position;
uniform vec2 Size;
uniform vec2 TextureOffset;
uniform vec2 TextureSize;

void main(void)
{
    vec2 TransformedPosition=(VPosition*Size)+Position;
    gl_Position =vec4(TransformedPosition.x/Viewport.x*2-1,-TransformedPosition.y/Viewport.y*2+1,0,1.0);
    FTexCoord = VTexCoord*TextureSize+TextureOffset;
}
