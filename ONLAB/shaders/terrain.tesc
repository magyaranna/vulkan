#version 450


layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;


layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} u;

layout (vertices = 4) out;
 
layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out vec3 outNormal[4];
layout (location = 1) out vec2 outUV[4];


void main()
{

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];

    /* if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
        gl_TessLevelOuter[3] = 1;

        gl_TessLevelInner[0] = 1;
        gl_TessLevelInner[1] = 1;
    }*/
    

    if(gl_InvocationID == 0)
    {

        const int MIN_TESS_LEVEL = 1;
        const int MAX_TESS_LEVEL = 5;
        const float MIN_DISTANCE = 0;
        const float MAX_DISTANCE = 80;


        vec4 viwePos00 = g.view * u.model * gl_in[0].gl_Position;
        vec4 viwePos01 = g.view * u.model * gl_in[1].gl_Position;
        vec4 viwePos10 = g.view * u.model * gl_in[2].gl_Position;
        vec4 viwePos11 = g.view * u.model * gl_in[3].gl_Position;

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
    }
} 