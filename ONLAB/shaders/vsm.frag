
#version 450

layout(location = 0) in vec4 fragPosLightSpace;
layout(location = 0) out vec2 fragMoment;


void main() {
	
 
    float depth = fragPosLightSpace.z / fragPosLightSpace.w;
	depth = depth * 0.5 + 0.5;
	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25 * (dx * dx + dy * dy);
	fragMoment = vec2(moment1, moment2);
	
}
