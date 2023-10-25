
#version 450

layout(location = 0) in vec4 fragPosLightSpace;
layout(location = 0) out vec2 outMoment;


void main() {
	
 
    float depth = fragPosLightSpace.z / fragPosLightSpace.w;
	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25 * (dx * dx + dy * dy);
	outMoment = vec2(moment1, moment2);
	
}
