#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_clip_cull_distance: enable

layout(location = 0) out vec2 uv;

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { 
    mat4 view;
    mat4 proj;
} global;

layout( push_constant ) uniform pushConstants {
	layout(offset = 32) vec4 clipPlane; 
} p;




void main() 
{
	uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(uv * 2.0f - 1.0f, 0.0f, 1.0f);

	mat4 invViewProj = inverse(global.proj * global.view);
    vec3 clipSpace = vec3(uv*vec2(2.0) - vec2(1.0), 1.0f);  
 
    vec4 worldPos = invViewProj * vec4(clipSpace, 1.0);  
	vec3 pos = worldPos.xyz/ worldPos.w;
	

	gl_ClipDistance[0] = dot(vec4(worldPos) ,p.clipPlane);
} 
