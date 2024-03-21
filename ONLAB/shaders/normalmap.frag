#version 450


layout(set = 0, binding = 0) uniform sampler2D heightmap;


layout (location = 0) in vec2 uv;

layout (location = 0) out vec3 outFragColor;


void main()
{             
	vec2 texelSize = 1.0 / textureSize(heightmap, 0);

	 float heights[3][3];
	 for (int sx = -1; sx <= 1; sx++) {
		for (int sy = -1; sy <= 1; sy++) {
			vec2 rpos = vec2(uv.x + sx * texelSize.x, uv.y + sy * texelSize.y);
			rpos.x = max(0, min(rpos.x, textureSize(heightmap, 0).x - 1));
			rpos.y = max(0, min(rpos.y, textureSize(heightmap, 0).y - 1));
						
			heights[sx + 1][sy + 1] = texture(heightmap, rpos).r ;
		}
	}

	vec3 normal;

	normal.x = heights[0][0] - heights[2][0] + 2.0f * heights[0][1] - 2.0f * heights[2][1] + heights[0][2] - heights[2][2];
	normal.z = heights[0][0] + 2.0f * heights[1][0] + heights[2][0] - heights[0][2] - 2.0f * heights[1][2] - heights[2][2];	
	normal.y = 0.25f * sqrt(1.0f - normal.x * normal.x - normal.z * normal.z);

    outFragColor = normalize(normal);
} 
