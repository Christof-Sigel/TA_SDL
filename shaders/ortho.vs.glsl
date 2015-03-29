#version 400

layout(location=0) in vec2 VPosition;
layout(location=1) in vec2 VTexCoord;

noperspective out vec2 FTexCoord;
noperspective out vec2 FVertCoord;

uniform ivec2 Viewport;

void main(void)
{
   //    FPosition=(ModelViewMatrix*vec4(VPosition,1.0)).xyz;
    FTexCoord=vec2(VTexCoord.x,VTexCoord.y);
    FVertCoord =vec2(VPosition.x/Viewport.x*2-1,-VPosition.y/Viewport.y+1);
    gl_Position=vec4(FVertCoord,0,1.0);
}
