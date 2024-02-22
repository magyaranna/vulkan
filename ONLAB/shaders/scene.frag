#version 450

layout(set = 5, binding = 0) uniform sampler2D shadowSampler;
layout(set = 6, binding = 0) uniform sampler2DArray  cascadeShadowSampler;
layout(set = 8, binding = 0) uniform sampler2D VSMshadowSampler;
layout(set = 9, binding = 0) uniform sampler2D ESMshadowSampler;


layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 0) uniform sampler2D normalMap;


layout(set = 4, binding = 0) uniform UniformBufferLightVP {
    mat4 view;
    mat4 proj;
    vec3 pos;
    vec3 dir;
} light;


layout (set = 7, binding = 5) uniform UniformLightSpace {
	mat4 cascadeViewProjMat[4];
    vec4 cascadeSplits;
} u;

layout( push_constant ) uniform pushConstants {
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
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 worldPos;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 outColor;




float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    float shadow = 0.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    
    float bias = 0.0;
    if(p.bias == 1){
        bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005) * 0.1; 
    }

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

        float distToLight = texture(shadowSampler, projCoords.xy).r;
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
 

float chebyshevUpperBound( vec4 fragPosLightSpace, int index) {    /*VSM*/

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if(p.cascade == 1){

         vec2 moments = texture(cascadeShadowSampler, vec3(projCoords.st, index)).xy;

         if ( fragPosLightSpace.z > -1.0 && fragPosLightSpace.z < 1.0 ) {
	        if ( fragPosLightSpace.w > 0 && moments.x < fragPosLightSpace.z - 0.000) {
			     float variance = moments.y - (moments.x * moments.x); 
                variance = max(variance, 0.0002);
                float d = projCoords.z - moments.x;
   
                float p_max = linstep(0.0, 1.0, variance / (variance + d * d ));

                return p_max;
	        }
        }
    }
    else{
        vec2 moments = texture(VSMshadowSampler, projCoords.xy).xy;

        if (projCoords.z <= moments.x)
            return 1.0;

        float variance = moments.y - (moments.x * moments.x); 
        variance = max(variance, 0.0002);
        float d = projCoords.z - moments.x;
   
        float p_max = linstep(0.0, 1.0, variance / (variance + d * d ));

        return p_max;
    }
    return 1.0;
}  



float cascadeShadowCalculation(uint cascadeIndex, vec2 offset, vec3 normal, vec3 lightDir)
{
    vec4 fragPosLightSpace  =  u.cascadeViewProjMat[cascadeIndex] * vec4(worldPos, 1.0);	
    fragPosLightSpace = fragPosLightSpace / fragPosLightSpace.w;
    
    vec2 projCoords = fragPosLightSpace.xy * 0.5 + 0.5;
    
    float shadow = 1.0;
	float bias = 0.0;
    if(p.bias == 1){
        bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005) * 0.1; 
    }

	if ( fragPosLightSpace.z > -1.0 && fragPosLightSpace.z < 1.0 ) {
	    float dist = texture(cascadeShadowSampler, vec3(projCoords.st + offset, cascadeIndex)).r;
	    if ( fragPosLightSpace.w > 0 && dist < fragPosLightSpace.z - bias) {
			    shadow = 0.3;
	    }
    }
	
	return shadow;
}

float cascadeFilterPCF(uint cascadeIndex, vec3 normal, vec3 lightDir)
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
			shadowFactor += cascadeShadowCalculation(cascadeIndex, vec2(dx*x, dy*y), normal, lightDir);
			count++;
		}
	}
	return shadowFactor / count;
}



void main() {

    /*Shading*/
    vec3 normal = texture(normalMap, uv).rgb;   
    normal = normalize(normal * 2.0 - 1.0); 
    normal = normalize(TBN * normal);

    vec3 lightDir   = normalize(light.pos - fragPos);

    vec3 lightColor = vec3(1.0f,1.0f,1.0f);
    float ambientStrength = 0.5;  
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = max(dot(normal,lightDir),0.0) * lightColor;   

    float specularStrength = 0.5;
    vec3 viewDir    = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal); 
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * specularStrength *lightColor;



    /*COLOR*/
    vec4 color = vec4(1.0);
    if(p.displayNormalmap == 1){
        if(p.esm == 1){
            color = vec4(fragNormal, 1.0);
         }
         else{
            color = vec4(normal, 1.0);
         }
        
    }else{
        if(p.vsm == 1){
             color = texture(texSampler, uv);
        }
        else{
             color = texture(texSampler, uv) * vec4((ambient +  diffuse), 1.0);
        }
       
    }
    
    /*Shadow*/

    float shadow = 0;
    vec4 fragPosLightSpace = light.proj * light.view * vec4(worldPos,1.0);

	if(p.cascade == 1 ){

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
        vec4 lightSpacePos = u.cascadeViewProjMat[cascadeIndex] * vec4(worldPos, 1.0);

        if(p.vsm == 1){
            shadow = chebyshevUpperBound(lightSpacePos, cascadeIndex);
        }
        else if(p.esm == 1){
            lightSpacePos.z = lightSpacePos.z / lightSpacePos.w;  

            vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
            projCoords.xy = projCoords.xy * 0.5 + 0.5;
            float e = texture(cascadeShadowSampler, vec3(projCoords.xy , cascadeIndex)).r;
           
            shadow = clamp( e * exp( -80.0f * lightSpacePos.z ), 0.2, 1.0);
        }
        else{  
            
            if(p.pcf == 1){
                shadow = cascadeFilterPCF(cascadeIndex, normal, lightDir);
            }
            else{
                shadow = cascadeShadowCalculation(cascadeIndex, vec2(0.0, 0.0), normal, lightDir);
            }
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

            shadow = chebyshevUpperBound(fragPosLightSpace, -1);
        } 
         else if(p.esm == 1){
            fragPosLightSpace.z = fragPosLightSpace.z / fragPosLightSpace.w;    //d

            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords.xy = projCoords.xy * 0.5 + 0.5;
            float x = texture(ESMshadowSampler, projCoords.xy).r;
            
            shadow = clamp( x * exp( -80.0f * fragPosLightSpace.z ), 0.3, 1.0);
        }
        else{
            shadow = ShadowCalculation(fragPosLightSpace, normal,lightDir);

        }
    }


    float alpha = texture(texSampler, uv).a;
	if (alpha < 0.5) {
	    	discard;
	}

    

    outColor = vec4(/*shadow **/ color.rgb , 1.0 );
    
}

