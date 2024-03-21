#version 450


layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;


layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} u;

layout( push_constant ) uniform pushConstants {
  layout(offset = 32) 
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

	/* if(gl_InvocationID == 0)
    {

        const int MIN_TESS_LEVEL = 1;
        const int MAX_TESS_LEVEL = 30;
        const float MIN_DISTANCE = 20;
        const float MAX_DISTANCE = 100;


        vec4 viwePos00 = g.view * gl_in[0].gl_Position;
        vec4 viwePos01 = g.view * gl_in[1].gl_Position;
        vec4 viwePos10 = g.view * gl_in[2].gl_Position;
        vec4 viwePos11 = g.view * gl_in[3].gl_Position;

        float distance00 = clamp((abs(viwePos00.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance01 = clamp((abs(viwePos01.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance10 = clamp((abs(viwePos10.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance11 = clamp((abs(viwePos11.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);


        float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
        float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
        float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
        float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );


        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }*/

} 