#version 400

in vec3 GNormal;
in vec3 GPosition;
noperspective in vec3 GEdgeDistance;
//flat in int GIsEdge;
layout (location =0 ) out vec4 out_Color;


uniform vec3 MeshColor;


const int levels=4;
const float scaleFactor=1.0/levels;


uniform float LineWidth;
uniform vec3 LineColor;

vec3 LightPos[2]=vec3[](vec3(0,0,-8),vec3(50,50,20));
void main(void)
{
    
    float level=0;
    for(int i=0;i<LightPos.length;i++)
    {
	vec3 s = normalize(LightPos[i] - GPosition);
	float cosine =max(0,dot(s,GNormal));
    	float templevel=floor(cosine * levels)*scaleFactor;
	level+=templevel;
    }
    
    float d=min(GEdgeDistance.x,GEdgeDistance.y);
    d=min(d,GEdgeDistance.z);
    float mixVal=smoothstep(LineWidth-1,LineWidth+1,d);
    
    
    out_Color = vec4(mix(LineColor,level*MeshColor,mixVal),1.0);
}
