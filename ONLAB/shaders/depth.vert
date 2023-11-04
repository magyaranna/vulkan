#version 450

layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} ubo;
	

layout( set = 2, binding = 0) uniform UniformBufferLightVP {	
   mat4 view;
   mat4 proj;
} light;

layout(set = 2, binding = 1) uniform UniformBufferLight {
    vec3 pos;
	vec3 dir;
} lbo;

layout(location = 0) in vec3 inPosition;	
layout(location = 1) in vec2 inUV;	

layout (location = 0) out vec2 outUV;

void main() {

	outUV = inUV;
	gl_Position = light.proj * light.view * ubo.model * vec4(inPosition, 1.0);   
	
}