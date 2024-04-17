#version 450

layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} obj;
	
layout(set = 2, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} global;


layout(location = 0) in vec3 inPosition;	
layout(location = 1) in vec2 inUV;	

layout (location = 0) out vec2 outUV;

void main() {

	outUV = inUV;
	gl_Position = global.proj * global.view * obj.model * vec4(inPosition, 1.0);  
	
}