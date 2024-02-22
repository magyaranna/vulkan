#version 450

layout (location = 0) in vec3 inPos;


layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} global;


layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;

	mat4 viewMat = mat4(mat3(global.view));
	gl_Position = global.proj * viewMat * vec4(inPos.xyz, 1.0);

}