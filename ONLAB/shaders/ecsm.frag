#version 450


layout(set = 0, binding = 0) uniform sampler2DArray shadowSampler;

layout( push_constant ) uniform PushConstants {
  uint cascadeIndex;
} p;


layout (location = 0) in vec2 uv;

layout (location = 0) out vec2 outFragColor;


void main()
{             
   vec2 texelSize = 1.0 / textureSize(shadowSampler, 0).xy;
    

	float x =  exp(80.0 * texture(shadowSampler, vec3(uv + vec2(0,0) * texelSize, p.cascadeIndex)).r );
	 x +=  exp(80.0 * texture(shadowSampler, vec3(uv + vec2(0,1) * texelSize, p.cascadeIndex)).r );
	 x +=  exp(80.0 * texture(shadowSampler, vec3(uv + vec2(1,0) * texelSize, p.cascadeIndex)).r );
	 x +=  exp(80.0 * texture(shadowSampler, vec3(uv + vec2(1,1) * texelSize, p.cascadeIndex)).r );

	 x /= 4.0f;

   outFragColor = vec2( x , 0.0);
} 

