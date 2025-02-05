
#version 450

layout(set = 0, binding = 0) uniform sampler2D shadowSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec2 outMoment;


vec2 ComputeMoments(float t) {  
	vec2 Moments;  
	Moments.x = t;  

	float dx = dFdx(t);   
	float dy = dFdy(t);   
		
	Moments.y = t*t + 0.25*(dx*dx + dy*dy);  
	return Moments; 
 } 


void main() {
	
	float depth = texture(shadowSampler, uv).r; 
	
	outMoment = ComputeMoments(depth);
}
