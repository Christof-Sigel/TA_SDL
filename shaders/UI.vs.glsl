#version 400

layout(location=0) in vec2 VPosition;
layout(location=1) in vec4 VEdgeDistance;

noperspective out vec4 FEdgeDistance;

uniform ivec2 Viewport;
uniform vec2 Position;
uniform vec2 Size;

void main(void)
{
    vec2 TransformedPosition=(VPosition*Size)+Position;
    FEdgeDistance=VEdgeDistance;
    FEdgeDistance=vec4(VEdgeDistance.x * Size.y,VEdgeDistance.z*Size.y,VEdgeDistance.y*Size.x,VEdgeDistance.w*Size.x);
    gl_Position =vec4(TransformedPosition.x/Viewport.x*2-1,-TransformedPosition.y/Viewport.y*2+1,0,1.0);
}

