#version 450


layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { 
    mat4 view;
    mat4 proj;
} g;

layout(set = 2,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} u; 

layout( push_constant ) uniform pushConstants {
 // layout(offset = 24) 
  vec2 viewport;
  float tessfactor;
} p;


layout (vertices = 4) out;
 
layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out vec3 outNormal[4];
layout (location = 1) out vec2 outUV[4];


float tesselationFactor(vec4 p0, vec4 p1)
{
	vec4 midPoint = 0.5 * (p0 + p1);
	
	float radius = distance(p0, p1) / 2.0;
	vec4 v0 = g.view * midPoint;
	
	vec4 clip0 = (g.proj * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (g.proj * (v0 + vec4(radius, vec3(0.0))));

	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= p.viewport;
	clip1.xy *= p.viewport;
	
	return clamp(distance(clip0, clip1) / p.tessfactor, 1.0, 64.0);
}



void main()
{

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];

       
     if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tesselationFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
		gl_TessLevelOuter[1] = tesselationFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
		gl_TessLevelOuter[2] = tesselationFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
		gl_TessLevelOuter[3] = tesselationFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
		gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
		gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
    }

} 