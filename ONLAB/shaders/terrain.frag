#version 450

//layout(set = 3, binding = 4) uniform sampler2DShadow shadowSampler;
layout(set = 3, binding = 4) uniform sampler2D shadowSampler;
layout(set = 4, binding = 7) uniform sampler2DArray  cascadeShadowSampler;
//layout(set = 6, binding = 9) uniform sampler2D VSMshadowSampler;
layout(set = 6, binding = 11) uniform sampler2D VSMshadowSampler;
layout(set = 7, binding = 10) uniform sampler2D ESMshadowSampler;

layout(set = 2, binding = 2) uniform UniformBufferLightVP {
    mat4 view;
    mat4 proj;
} light;

layout(set = 2, binding = 3) uniform UniformBufferLight {
    vec4 pos;
    vec3 dir;
} lbo;

layout (set = 5, binding = 8) uniform UniformLightSpace {
	mat4 cascadeViewProjMat[4];
    vec4 cascadeSplits;
} u;

layout( push_constant ) uniform pushConstants {
  int displayNormalmap;
  int cascade;
  int vsm;
  int esm;
  int cascadecolor;
  int cascadePCF;
  int pcf;
  int bias;
} p;



layout(location = 0) in vec3 viewPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 inPos;
layout(location = 3) in vec3 worldPos;

layout(location = 0) out vec4 outColor;


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);


/*
float ShadowCalculation(vec4 fposLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fposLightSpace.xyz / fposLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    {
     float bias = 0.0;
     bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
     projCoords.z -= bias * 0.1;
    }
    float shadow = texture(shadowSampler, projCoords.xyz).r;

  
    if(shadow == 0.0f){
         return 0.3f;
    }
    else
         return 1.0f;
}  */

float ShadowCalculation(vec4 fposLightSpace, vec3 normal, vec3 lightDir)
{

    float shadow = 0.0;

    vec3 projCoords = fposLightSpace.xyz / fposLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    
    float bias = 0.0;
    if(p.bias == 1){
        bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005) * 0.1; 
    }

    
    //projCoords.z -= bias * 0.1;
    
    float distToLight = texture(shadowSampler, projCoords.xy).r;

    if(p.pcf == 1){
                vec2 texelSize = 1.0 / textureSize(shadowSampler, 0);
                for(int x = -1; x <= 1; ++x)
                {
                    for(int y = -1; y <= 1; ++y)
                    {
                        float pcfDepth = texture(shadowSampler, projCoords.xy + vec2(x, y) * texelSize).r; 
                        shadow += projCoords.z - bias < pcfDepth ? 1.0 : 0.3;        
                    }    
                }
                shadow /= 9.0;
     }
     else{
        if (projCoords.z - bias <= distToLight && projCoords.z < 1.0)
            shadow = 1.0;
        
        else
            shadow = 0.3;
        
     }
     return shadow;

} 


 float linstep(float min, float max, float v) {   
    return clamp((v-min) / (max-min), 0.2, 1.0);
 
 } 
 

float chebyshevUpperBound( vec4 fragPosLightSpace) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

  //  float surfaceDistToLight = length(lbo.pos.xyz - worldPos);

    vec2 moments = texture(VSMshadowSampler, projCoords.xy).xy;  //E(x) 
 
    if (projCoords.z <= moments.x)
        return 1.0;

    float variance = moments.y - (moments.x * moments.x); //D^2(x)
    variance = max(variance, 0.0002);
    float d = projCoords.z - moments.x;
   
   // float p_max = clamp(variance / (variance + d * d),0.2,0.8);       ///ez
   // float p_max = variance / (variance + d * d);

   float p_max = linstep(0.0, 1.0, variance / (variance + pow(d * d , 1)));

    return p_max;
}  



float cascadeShadowCalculation(uint cascadeIndex, vec2 offset)
{
	
    vec4 fragPosLightSpace  =  u.cascadeViewProjMat[cascadeIndex] * vec4(worldPos, 1.0);	
    fragPosLightSpace = fragPosLightSpace / fragPosLightSpace.w;
    
    vec2 projCoords = fragPosLightSpace.xy * 0.5 + 0.5;
    
    float shadow = 1.0;
	float bias = 0.005;

	if ( fragPosLightSpace.z > -1.0 && fragPosLightSpace.z < 1.0 ) {
	    float dist = texture(cascadeShadowSampler, vec3(projCoords.st + offset, cascadeIndex)).r;
	    if ( fragPosLightSpace.w > 0 && dist < fragPosLightSpace.z - bias) {
			    shadow = 0.3;
	    }
    }
	
	return shadow;
}

float cascadeFilterPCF(uint cascadeIndex)
{
	ivec2 texDim = textureSize(cascadeShadowSampler, 0).xy;
	float scale = 0.75;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += cascadeShadowCalculation(cascadeIndex, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadowFactor / count;
}

void main(){

	vec3 normal = normalize(fragNormal);
	float shading = dot(normal, vec3(0.0, 1.0, 0.0));
    vec3 green = vec3(0.416,0.529,0.369);

    vec4 fragPosLightSpace = light.proj * light.view * vec4(worldPos,1.0);   
    vec3 lightDir = normalize(lbo.dir); 

    /*COLOR*/
    vec3 color = vec3(1.0);
    if(p.displayNormalmap == 1){
        color = normal;
    }else{
       color = green * shading;
    }

    /*Shadow*/
    float shadow = 0;


	if(p.cascade == 1){

        float depthValue = abs(viewPos.z);
        int cascadeIndex = 3;
        for (int i = 0; i < 3; ++i)
        {
            if (depthValue < u.cascadeSplits[i])
            {
                cascadeIndex = i;
                break;
            }
        }

        if(p.cascadePCF == 1){
             shadow = cascadeFilterPCF(cascadeIndex);
        }
        else{
             shadow = cascadeShadowCalculation(cascadeIndex, vec2(0.0, 0.0));
        }


        if(p.cascadecolor == 1){
            switch(cascadeIndex) {
			case 0 : 
				color.rgb *= vec3(1.0f, 0.25f, 0.25f);
				break;
			case 1 : 
				color.rgb *= vec3(0.25f, 1.0f, 0.25f);
				break;
			case 2 : 
				color.rgb *= vec3(0.25f, 0.25f, 1.0f);
				break;
			case 3 : 
				color.rgb *= vec3(1.0f, 1.0f, 0.25f);
				break;
	        }
        }
    }
    else{

        if(p.vsm == 1){

            shadow = chebyshevUpperBound(fragPosLightSpace);
        } 
         else if(p.esm == 1){
            float t = fragPosLightSpace.z / fragPosLightSpace.w;    //d

            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords.xy = projCoords.xy * 0.5 + 0.5;
            float nearestOccluderDepth = texture(ESMshadowSampler, projCoords.xy).r;
            
            shadow = clamp( exp( -80.0f * ( t- nearestOccluderDepth ) ), 0.0, 1.0 );  //e^(-c*(d-z))
            
        }
        else{
            shadow = ShadowCalculation(fragPosLightSpace, normal,lightDir);


           
        }
    }

    outColor = vec4( shadow* color.rgb,1.0 );
    
	
}


