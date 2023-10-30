
#version 450

layout(location = 0) in vec4 fragPosLightSpace;
layout(location = 1) in vec4 viewPos;

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
	
	float DistToLight = length(viewPos);   
   
	float depth = fragPosLightSpace.z; // fragPosLightSpace.w;
	
	outMoment = ComputeMoments(depth);
}
