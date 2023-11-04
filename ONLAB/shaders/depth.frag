#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;

void main()
{             
   float alpha = texture(texSampler, inUV).a;
	if (alpha < 0.5) {
		discard;
	}
  
} 
