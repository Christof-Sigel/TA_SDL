#version 400

layout(location=0) in vec3 VPosition;
layout(location=1) in vec2 VTexCoord;
layout(location=2) in vec3 VColor;

out vec3 FPosition;
out vec3 FColor;
smooth out vec2 FTexCoord;

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

void main(void)
{
     FColor=VColor;
   //    FPosition=(ModelViewMatrix*vec4(VPosition,1.0)).xyz;
    FPosition=vec3(VTexCoord,1);
    FTexCoord=VTexCoord;
    gl_Position=ProjectionMatrix*(ModelViewMatrix*vec4(VPosition,1.0));
}
