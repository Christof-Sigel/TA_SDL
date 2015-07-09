#version 4000

layout(location=0) in vec2 VPosition;

noperspective out vec2 FTexCoord;

uniform ivec2 Viewport;
uniform vec2 Position;
uniform vec2 Size;


out vec2 Dimension;
out vec2 FTexCoord;

void main(void)
{
    vec2 TransformedPosition=(VPosition*Size)+Position;
    gl_Position =vec4(TransformedPosition.x/Viewport.x*2-1,-TransformedPosition.y/Viewport.y*2+1,0,1.0);
    Dimension = Size;
}
