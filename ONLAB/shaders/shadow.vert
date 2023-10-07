#version 450

layout(set = 0,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} ubo;
	

layout( set = 1, binding = 2) uniform UniformBufferLightVP {	
   mat4 view;
   mat4 proj;
} light;

layout(set = 1, binding = 3) uniform UniformBufferLight {
    vec4 pos;
} lbo;

layout(location = 0) in vec3 inPosition;	

void main() {

	gl_Position = light.proj * light.view * ubo.model * vec4(inPosition, 1.0);   
	
}