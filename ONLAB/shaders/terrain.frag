#version 450



/*layout(set = 4, binding = 0) uniform sampler2DArray  cascadeShadowSampler;  
layout (set = 5, binding = 5) uniform UniformLightSpace { 
	mat4 cascadeViewProjMat[4];
    vec4 cascadeSplits;
} u;
layout(set = 6, binding = 0) uniform sampler2D VSMshadowSampler;  
layout(set = 7, binding = 0) uniform sampler2D ESMshadowSampler; 
layout(set = 8, binding = 0) uniform sampler2D shadowSampler;  */


layout (set = 0, binding = 0) uniform sampler2D grassTexture; 


layout (set = 4, binding = 0) uniform sampler2D normalMap; 
layout(set = 5, binding = 0) uniform UniformBufferLightVP {
    mat4 view;
    mat4 proj;
    vec3 pos;
    vec3 dir;
} light;
layout (set = 6, binding = 0) uniform sampler2D snowTexture; 
layout (set = 7, binding = 0) uniform sampler2D sandTexture;  
layout (set = 8, binding = 0) uniform sampler2D rockTexture;  



layout(set = 1, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} g;







layout( push_constant ) uniform pushConstants {
layout(offset = 36)
  int displayNormalmap;
  int cascade;
  int vsm;
  int esm;
  int cascadecolor;
  int pcf;
  int bias;
} p;


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec2 inUV;
layout(location = 5) in float height;


layout(location = 0) out vec4 outColor;



vec3 reflection;
vec3 shading;
float alpha = 30.0;
float snow_height = 2000.0; //120.0;
float grass_height = 40.0; //12.0;
float mix_zone = 25.0;//12.0;

void main(){

    vec3 normal =  texture(normalMap, inUV).rgb;
    float shading = dot(normal, vec3(0.0, 1.0, 0.0));

    vec4 fragPosLightSpace = light.proj * light.view * vec4(worldPos,1.0);   
    vec3 lightDir = normalize(light.pos - fragPos);

    /*COLOR*/
    vec3 color = vec3(1.0);
    if(p.displayNormalmap == 1){
        color = normal;
    }else{
        
        vec3 snow = texture(snowTexture, inUV*10).rgb;
        vec3 grass = texture(grassTexture, inUV*30).rgb;
        vec3 sand = texture(sandTexture, inUV*15).rgb;
        vec3 rock = texture(rockTexture, inUV*40).rgb;  
        float angleDiff = abs(dot(normal.xyz, vec3(0, 1, 0)));
        float pureRock = 0.5;
        float lerpRock = 0.9;
        float coef = 1.0 - smoothstep(pureRock, lerpRock, angleDiff);
        grass = mix(grass, rock, coef);
        snow = mix(snow, rock, coef);
        coef = smoothstep(0.80, 0.98, angleDiff);
       // grass = mix(grass, snow, coef);

        
        if (height > snow_height + mix_zone){
            color = snow;
        } else if (height > snow_height - mix_zone) {
            float coef = (height-(snow_height - mix_zone))/(2.0 * mix_zone);
            color = mix(grass, snow, coef);
        } else if (height > grass_height + mix_zone){
            color = grass;
        } else if (height > grass_height - mix_zone){
            float coef = (height-(grass_height - mix_zone))/(2.0 * mix_zone);
            color = mix(sand, grass, coef);
        } else {
            color = sand;
        }
    }



    /*Shadow*/
    float shadow = 0;

	

    outColor = vec4( color.rgb ,1.0 );
    //outColor = vec4( normal ,1.0 );
	//outColor = vec4( vec3(height, height, height) ,1.0 );
  //  outColor = vec4( 0.0, 0.0, 0.0 ,1.0 );
}


