#version 450

layout (set = 0, binding = 9) uniform sampler2D samplerColor;

layout( push_constant ) uniform pushConstants {
  int blurdirection;
} p;


layout (location = 0) in vec2 uv;

layout (location = 0) out vec2 outFragColor;

void main() 
{
	float weight[5];
	weight[0] = 0.227027;
	weight[1] = 0.1945946;
	weight[2] = 0.1216216;
	weight[3] = 0.054054;
	weight[4] = 0.016216;
	

	vec2 tex_offset = 1.0 / textureSize(samplerColor, 0); 

	vec2 fragmentColor = texture(samplerColor, uv).xy * weight[0]; 

	for(int i = 1; i < 5; ++i)
	{
		if (p.blurdirection == 1)
		{
			// H
			fragmentColor += texture(samplerColor, uv + vec2(tex_offset.x * i, 0.0)).xy * weight[i];
			fragmentColor += texture(samplerColor, uv - vec2(tex_offset.x * i, 0.0)).xy * weight[i];
		}
		else
		{
			// V
			fragmentColor += texture(samplerColor, uv + vec2(0.0, tex_offset.y * i)).xy * weight[i];
			fragmentColor += texture(samplerColor, uv - vec2(0.0, tex_offset.y * i)).xy * weight[i];
		}
	}
	outFragColor = fragmentColor;
}




