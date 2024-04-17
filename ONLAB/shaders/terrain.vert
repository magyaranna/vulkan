#version 450


layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { 
    mat4 view;
    mat4 proj;
} g;


layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;


layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;

void main(){
    
    gl_Position = vec4(inPos.xyz, 1.0);
	outNormal = inNormal;
    outUV = inUV;
}