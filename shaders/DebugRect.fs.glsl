#version 400

noperspective in vec4 FEdgeDistance;
out vec4 out_Color;

uniform vec4 Color;
uniform vec4 BorderColor;
uniform float BorderWidth;

uniform ivec2 Viewport;
in vec2 Dimension;

void main(void)
{

    float d1 = min(FEdgeDistance.x,FEdgeDistance.y);
    float d2 =min(FEdgeDistance.z,FEdgeDistance.w);

    out_Color=vec4(Color);
    if(d1 < BorderWidth || d2 < BorderWidth)
    {
	out_Color=vec4(BorderColor);
    }
}
