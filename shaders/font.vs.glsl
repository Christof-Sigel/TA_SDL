#version 400

layout(location=0) in vec2 VPosition;
layout(location=1) in vec2 VTexCoord;

noperspective out vec2 FTexCoord;

uniform ivec2 Viewport;
uniform vec2 Position;

void main(void)
{
   //    FPosition=(ModelViewMatrix*vec4(VPosition,1.0)).xyz;
    FTexCoord=vec2(VTexCoord.x,VTexCoord.y);
    vec2 NewPos = VPosition+Position;
    gl_Position = vec4(NewPos.x/Viewport.x*2-1,-NewPos.y/Viewport.y*2+1,0,1.0);
}
