#version 400

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform mat4 ProjectionMatrix;

out vec3 VPosition;
out vec3 VNormal;

uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
void main(void)
{
    VNormal=normalize(NormalMatrix*in_Normal);
    VPosition=(ModelMatrix*vec4(in_Position,1.0)).xyz;
    // VPosition=in_Position-vec3(0,0,2);
    gl_Position=ProjectionMatrix*(ModelMatrix*vec4(in_Position,1.0));
    // gl_Position=ProjectionMatrix*vec4(in_Position+vec3(0,0,-2),1.0);
    //gl_Position=in_Position;
}
