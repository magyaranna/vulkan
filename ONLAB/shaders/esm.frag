#version 450


layout(set = 2, binding = 4) uniform sampler2D shadowSampler;


layout(location = 0) in vec4 lightspacePos;
layout(location = 0) out vec2 outMoment;


void main()
{             

	vec2 texelSize = 1.0 / textureSize(shadowSampler, 0);
	
    float depth = lightspacePos.z / lightspacePos.w;

	vec3 projCoords = lightspacePos.xyz / lightspacePos.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

	/*
	float x = clamp(exp( -1000.0f * (depth - texture(shadowSampler, projCoords.xy + vec2(0,0) * texelSize ).r ))  , 0.0, 1.0);
	x += clamp(exp( -1000.0f * (depth - texture(shadowSampler, projCoords.xy + vec2(0,1) * texelSize ).r ))  , 0.0, 1.0);
	x += clamp(exp( -1000.0f * (depth - texture(shadowSampler, projCoords.xy + vec2(1,0) * texelSize ).r ))  , 0.0, 1.0);
	x += clamp(exp( -1000.0f * (depth - texture(shadowSampler, projCoords.xy + vec2(1,1) * texelSize ).r ))  , 0.0, 1.0);

	x /= 4.0f;
	*/
	float x = clamp(exp( -1001.0f * (depth - texture(shadowSampler, projCoords.xy ).r))  , 0.0, 1.0);

    
    outMoment = vec2( x , projCoords);
} 