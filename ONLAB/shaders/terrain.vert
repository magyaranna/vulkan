#version 450



layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;


layout(set = 1,  binding = 1) uniform UniformBufferObject {	
    mat4 model;  

} u;
	

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec3 viewPos;

void main(){
    
    fragPos = inPos;
    fragNormal = normal;
    viewPos = (g.view *vec4(inPos, 1.0)).xyz;  
    worldPos =  (u.model * vec4(inPos,1.0)).xyz;

    gl_Position = g.proj * g.view * u.model * vec4(inPos,1.0);
    
}