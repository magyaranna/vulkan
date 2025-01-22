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

	/*float waweStrength = 0.015;   eredeti
	vec2 distortion1 = (texture(dudvTexture, vec2(inUV.x, inUV.y+ p.moveFactor)*10 ) * 2.0 - 1.0).rg * waweStrength;
	vec2 distortion2 = (texture(dudvTexture, vec2(inUV.x, inUV.y)*10 ) * 2.0 - 1.0).rg  * waweStrength;
	*/
	/*float waweStrength = 0.017;     sima dudv
	vec2 distortion1 = (texture(dudvTexture, vec2(inUV.x, inUV.y+ p.moveFactor * 10.0f) ) * 2.0 - 1.0).rg * waweStrength;
	vec2 distortion2 = (texture(dudvTexture, vec2(inUV.x, inUV.y) *10.0f ) * 2.0 - 1.0).rg  * waweStrength;*/
	//float waweStrength = 0.010;
	float distanceFromCamera = length(viewVector);
	float waweStrength = 1.0 / ( distanceFromCamera * 8000.0);
	float waveIntensity1 = 2.0/ distanceFromCamera * 10.0f;
	float waveIntensity2 = exp(-distanceFromCamera * 0.5);
    
	float waveIntensity3 = clamp(1.0 - distanceFromCamera * 0.05, 0.0, 1.0);
	float waveIntensity4 = pow(max(0.0, 1.0 - distanceFromCamera * 0.2), 8.0);
    

	vec2 distortion1 = (texture(dudvTexture, vec2(inUV.x* waveIntensity2 , inUV.y+ p.moveFactor * 2.0f) ) * 2.0 - 1.0).rg * waweStrength ;
	vec2 distortion2 = (texture(dudvTexture, vec2(inUV.x* waveIntensity1, inUV.y * waveIntensity3) *waveIntensity4 ) * 2.0 - 1.0).rg  * waweStrength;
	vec2 distortion3 = (texture(dudvTexture, vec2(inUV.x, inUV.y * waveIntensity2) *waveIntensity4 ) * 2.0 - 1.0).rg  * waveIntensity4 *0.01;

	vec2 d = - distortion1 + distortion2 +distortion3*0.5;

	vec2 reflectionUV = clamp(vec2(ndc.x , 1 - ndc.y) + d, 0.001, 0.999);
	vec2 refractionUV = clamp(vec2(ndc.x, ndc.y) + d, 0.001, 0.999);

	vec4 reflection = texture(reflectionTexture, reflectionUV );
	vec4 refraction = texture(refractionTexture,refractionUV);

	float reflectionFactor = pow(dot(viewVector, vec3(0.0,1.0,0.0)), 0.5);

	vec4 color = mix(reflection, refraction, 0.6);
	outColor = vec4(mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.1).xyz, 1.0);
}