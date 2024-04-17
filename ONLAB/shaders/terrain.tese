#version 450

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { 
    mat4 view;
    mat4 proj;
} g;


layout(set = 2,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} ubo;


layout (set = 3, binding = 0) uniform sampler2D displacementMap; 

layout( push_constant ) uniform pushConstants {
 layout(offset = 12) 
  float displacementFactor;
  vec4 clipPlane;
} p;

#extension GL_EXT_clip_cull_distance: enable

layout(quads, equal_spacing  , cw) in;

layout (location = 0) in vec3 inNormal[];
layout (location = 1) in vec2 inUV[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 outWorldPos;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out vec2 outUV;
layout(location = 5) out float height;

void main()
{

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

	vec2 uv1 = mix(inUV[0], inUV[1], u);
	vec2 uv2 = mix(inUV[3], inUV[2], u);
	outUV = mix(uv1, uv2, v);

    height = texture(displacementMap, outUV).r * p.displacementFactor;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;
     
    vec4 pos1 = mix(p00, p01, u);
	vec4 pos2 = mix(p11, p10, u);
	vec4 pos = mix(pos1, pos2, v);
    pos.y += height;

   
    gl_Position = g.proj * g.view * ubo.model*  pos;

	fragPos = (pos / pos.w).xyz;
    viewPos = (g.view * ubo.model * pos).xyz;  
    vec4 worldPos = ubo.model * pos;

    gl_ClipDistance[0] = dot(worldPos ,p.clipPlane);

    outWorldPos = worldPos.xyz;
}