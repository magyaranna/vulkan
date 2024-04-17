#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;

layout( push_constant ) uniform pushConstants {
	layout(offset = 16)
	vec3 camPos; 
} p;

layout(location = 0) in vec3 inPosition;	
layout(location = 1) in vec2 inUV;	


layout (location = 0) out vec4 clipSpace;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 viewVector;

void main() {

	outUV = inUV * 6.0f;
	clipSpace = g.proj * g.view * vec4(inPosition, 1.0);
	viewVector = normalize(p.camPos - inPosition);
	gl_Position = clipSpace;
	
}