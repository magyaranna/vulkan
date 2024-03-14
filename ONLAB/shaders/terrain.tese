#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;


layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} ubo;


layout (set = 8, binding = 0) uniform sampler2D displacementMap; 

layout( push_constant ) uniform pushConstants {
  layout(offset = 28) float displacementFactor;
} p;


layout(quads, equal_spacing, cw) in;

layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out float height;

void main()
{

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

	vec2 uv1 = mix(inUV[0], inUV[1], u);
	vec2 uv2 = mix(inUV[3], inUV[2], u);
	vec2 outUV = mix(uv1, uv2, v);

    float height = texture(displacementMap, outUV).r * p.displacementFactor;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;
     
    vec4 pos1 = mix(p00, p01, u);
	vec4 pos2 = mix(p11, p10, u);
	vec4 pos = mix(pos1, pos2, v);
    pos.y -= height;

    //patchnormal
    p00.y = texture(displacementMap, inUV[0]).r * p.displacementFactor;
    p01.y = texture(displacementMap, inUV[1]).r* p.displacementFactor;
    p10.y = texture(displacementMap, inUV[2]).r* p.displacementFactor;
    p11.y = texture(displacementMap, inUV[3]).r* p.displacementFactor;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(uVec.xyz, vVec.xyz), 0) );

	
  
    gl_Position = g.proj * g.view * pos;

	fragPos = (pos / pos.w).xyz;
    viewPos = (g.view * ubo.model * pos).xyz;  
    worldPos =  (ubo.model * pos).xyz;


    fragNormal = normal.xyz;

    vec3 n1 = mix(inNormal[0], inNormal[1], gl_TessCoord.x);
	vec3 n2 = mix(inNormal[3], inNormal[2], gl_TessCoord.x);
	//fragNormal = mix(n1, n2, gl_TessCoord.y);
}