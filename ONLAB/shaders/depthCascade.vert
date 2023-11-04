#version 450


layout( push_constant ) uniform PushConstants {
  uint cascadeIndex;
} pushConstant;


layout (set = 2, binding = 5) uniform UniformLightSpace {
	mat4 cascadeViewProjMat[4];
	vec4 cascadeSplits;
} u;

layout(set = 1, binding = 1) uniform UniformBufferObject {
    mat4 model;
} ubo;


layout(location = 0) in vec3 inPosition;	
layout(location = 1) in vec2 inUV;	

layout (location = 0) out vec2 outUV;

void main() {

	outUV = inUV;
	gl_Position =  u.cascadeViewProjMat[pushConstant.cascadeIndex] * ubo.model * vec4(inPosition, 1.0);       //1 * ubo.model 
}