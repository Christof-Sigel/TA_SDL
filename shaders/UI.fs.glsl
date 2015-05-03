#version 400

noperspective in vec4 FEdgeDistance;
out vec4 out_Color;

uniform vec3 Color;
uniform vec3 BorderColor;
uniform float BorderWidth;

uniform ivec2 Viewport;
uniform float Alpha;
in vec2 Dimension;

void main(void)
{

    float d1 = min(FEdgeDistance.x,FEdgeDistance.y);
    float d2 =min(FEdgeDistance.z,FEdgeDistance.w);
    // float mixVal = smoothstep(0,BorderWidth,d);

    //out_Color=vec4(mix(BorderColor,Color,mixVal),1.0);
    out_Color=vec4(Color,Alpha);
    float Gradient=1-d1/Dimension.y;
    Gradient = ((Gradient)+(1-d2/Dimension.x))/2;
//    Gradient = 1-Gradient;
    if(d1 < BorderWidth || d2 < BorderWidth)
    {
	out_Color=vec4(BorderColor*Gradient,Alpha);
    }
    // out_Color=vec4(d1,d2,1,1);
//    out_Color=vec4(FEdgeDistance.xyz,1);
}
