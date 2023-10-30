#version 450


layout(set = 0, binding = 4) uniform sampler2D shadowSampler;


layout (location = 0) in vec2 uv;

layout (location = 0) out vec2 outFragColor;


void main()
{             
	
	//vec4 x =  clamp(exp( 80.0f *  textureGatherOffset(shadowSampler, uv, ivec2(0,0), 0) ), 0.0, 1.0);

//	x +=  clamp(exp( 80.0f *  textureGatherOffset(shadowSampler, uv, ivec2(0,2), 0) ), 0.0, 1.0);
///	x +=  clamp(exp( 80.0f *  textureGatherOffset(shadowSampler, uv, ivec2(2,0), 0) ), 0.0, 1.0);
//	x +=  clamp(exp( 80.0f *  textureGatherOffset(shadowSampler, uv, ivec2(2,2), 0) ), 0.0, 1.0);

//	x /= 16.0f;
	vec4 x =  clamp(exp( -1.2f * textureGatherOffset(shadowSampler, uv, ivec2(0,0), 0) ), 0.0, 1.0);
    
    outFragColor = vec2( x.x , 0.0);
} 