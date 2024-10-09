#version 450

layout(set = 1, binding = 0) uniform sampler2D reflectionTexture;
layout(set = 2, binding = 0) uniform sampler2D refractionTexture;

layout(set = 3, binding = 0) uniform sampler2D dudvTexture;

layout( push_constant ) uniform pushConstants {
	float moveFactor; 
} p;

layout (location = 0) in vec4 clipSpace;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 viewVector;

layout (location = 0) out vec4 outColor;

void main()
{             

	vec2 ndc = (clipSpace.xy / clipSpace.w)/ 2.0 + 0.5;

	float waweStrength = 0.015;
	vec2 distortion1 = (texture(dudvTexture, vec2(inUV.x, inUV.y+ p.moveFactor)*20 ) * 2.0 - 1.0).rg * waweStrength;
	vec2 distortion2 = (texture(dudvTexture, vec2(inUV.x, inUV.y) *20 ) * 2.0 - 1.0).rg  * waweStrength;

	vec2 d = - distortion1 + distortion2;

	vec2 reflectionUV = clamp(vec2(ndc.x , 1 - ndc.y) + d, 0.001, 0.999);
	vec2 refractionUV = clamp(vec2(ndc.x, ndc.y) + d, 0.001, 0.999);

	vec4 reflection = texture(reflectionTexture, reflectionUV );
	vec4 refraction = texture(refractionTexture,refractionUV);

	float reflectionFactor = pow(dot(viewVector, vec3(0.0,1.0,0.0)), 0.5);

	vec4 color = mix(reflection, refraction, reflectionFactor);
	outColor = vec4(mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.3).xyz, 1.0);
} 
