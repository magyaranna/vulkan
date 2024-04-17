#version 450


layout(set = 0, binding = 0) uniform sampler2D depthBuffer;  

layout(location = 0) in vec2 uv;

layout (location = 0) out vec4 outFragColor;

void main() 
{

	float depth = texture(depthBuffer, uv).r;
	if(depth == 1.0){
		outFragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
	else{
		
		discard;
		//outFragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
	
}