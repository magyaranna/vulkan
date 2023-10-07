#version 450


layout( push_constant ) uniform PushConstants {
  uint cascadeIndex;
} pushConstant;


layout (set = 0, binding = 8) uniform UniformLightSpace {
	mat4 cascadeViewProjMat[4];
	vec4 cascadeSplits;
} u;

layout(set = 1, binding = 1) uniform UniformBufferObject {
    mat4 model;
} ubo;


layout(location = 0) in vec3 inPosition;	


void main() {

	gl_Position =  u.cascadeViewProjMat[pushConstant.cascadeIndex] * ubo.model * vec4(inPosition, 1.0);       //1 * ubo.model 
}