#version 450


layout(set = 0, binding = 0) uniform sampler2D computeResult;  

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { //
    mat4 view;
    mat4 proj;
} global;


layout(location = 0) in vec2 uv;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 color = texture(computeResult, uv);
	outFragColor = vec4(color);
	
}

