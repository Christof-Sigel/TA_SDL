#version 400

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 15) out;
		     
out vec3 GNormal;
out vec3 GPosition;
noperspective out vec3 GEdgeDistance;
in vec3 VNormal[];
in vec3 VPosition[];
uniform mat4 ViewPortMatrix;
uniform float LineWidth;

float CreaseThreshold=0.5f;
//flat out int GIsEdge;

const float PctExtend=0.05f;

bool IsFrontFacing(vec3 a, vec3 b, vec3 c)
{
    return ((a.x*b.y-b.x*a.y)+(b.x*c.y-c.x*b.y)+(c.x*a.y-a.x*c.y))>0;
}

bool IsCrease(vec3 a, vec3 b, vec3 c, vec3 n2)
{
    vec3 n1=normalize(cross(b-a,c-a));
    float dotn=dot(n1,n2);
    return dotn<CreaseThreshold;
}

void EmitEdgeQuad(vec3 e0, vec3 e1)
{
    vec3 ext=PctExtend*vec3(e1.xy - e0.xy,0);
    vec2 v = normalize(ext.xy);
    vec3 n = vec3(-v.y,v.x,0)*LineWidth/400;

    // GIsEdge=1;
    gl_Position=vec4(e0  + n - ext,1);
    EmitVertex();
    gl_Position=vec4(e0 - n - ext,1);
    EmitVertex();
    gl_Position=vec4(e1 +n + ext,1);
    EmitVertex();
    gl_Position=vec4(e1 - n + ext,1);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    vec3 p0=vec3(ViewPortMatrix*(gl_in[0].gl_Position/gl_in[0].gl_Position.w));
    vec3 p1=vec3(ViewPortMatrix*(gl_in[2].gl_Position/gl_in[2].gl_Position.w));
    vec3 p2=vec3(ViewPortMatrix*(gl_in[4].gl_Position/gl_in[4].gl_Position.w));

    vec3 p3=vec3(ViewPortMatrix*(gl_in[1].gl_Position/gl_in[1].gl_Position.w));
    vec3 p4=vec3(ViewPortMatrix*(gl_in[3].gl_Position/gl_in[3].gl_Position.w));
    vec3 p5=vec3(ViewPortMatrix*(gl_in[5].gl_Position/gl_in[5].gl_Position.w));
    
    float a=length(p1-p2);
    float b=length(p2-p0);
    float c=length(p1-p0);
    float alpha=acos((b*b+c*c-a*a)/(2.0*b*c));
    float beta=acos((a*a+c*c-b*b)/(2.0*a*c));
    float ha=abs(c*sin(beta));
    float hb=abs(c*sin(alpha));
    float hc=abs(b*sin(alpha));

    float EdgeOne=LineWidth*1.5f;
    float EdgeTwo=LineWidth*1.5f;
    float EdgeThree=LineWidth*1.5f;

    if(IsFrontFacing(p0,p1,p2))
     {
	vec3 n=normalize(cross(p1-p0,p2-p0));
	if(!IsFrontFacing(p1,p4,p2) ||IsCrease(p1,p4,p2,n))
	    EdgeOne=0;
	    //EmitEdgeQuad(p1,p2);
	if(!IsFrontFacing(p2,p5,p0) || IsCrease(p2,p5,p0,n))
	    // EmitEdgeQuad(p2,p0);
	    EdgeTwo=0;
	if(!IsFrontFacing(p0,p3,p1) || IsCrease(p0,p3,p1,n))
//	    EmitEdgeQuad(p0,p1);
	    EdgeThree=0;

     }

    //GIsEdge=0;
    
    GEdgeDistance = vec3(ha+EdgeOne,0+EdgeTwo,0+EdgeThree);
    GNormal=VNormal[0];
    GPosition=VPosition[0];
    gl_Position=gl_in[0].gl_Position;
    EmitVertex();

    GEdgeDistance = vec3(0+EdgeOne,hb+EdgeTwo,0+EdgeThree);
    GNormal=VNormal[2];
    GPosition=VPosition[2];
    gl_Position=gl_in[2].gl_Position;
    EmitVertex();

    GEdgeDistance = vec3(0+EdgeOne,0+EdgeTwo,hc+EdgeThree);
    GNormal=VNormal[4];
    GPosition=VPosition[4];
    gl_Position=gl_in[4].gl_Position;
    EmitVertex();

    EndPrimitive();

}
