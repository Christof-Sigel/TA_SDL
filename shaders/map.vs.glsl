#version 400

layout(location=0) in vec3 VPosition;
layout(location=1) in vec2 VTexCoord;


out vec2 FTexCoord;
out vec3 FPos;

uniform mat4 View;
uniform mat4 Projection;

void main(void)
{
    FTexCoord=VTexCoord;
    FPos=VPosition;
    gl_Position = Projection*(View*vec4(VPosition,1.0));
}
