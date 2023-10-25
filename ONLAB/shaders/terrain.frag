#version 450

//layout(set = 3, binding = 4) uniform sampler2DShadow shadowSampler;
layout(set = 3, binding = 4) uniform sampler2D shadowSampler;
layout(set = 4, binding = 7) uniform sampler2DArray  cascadeShadowSampler;
layout(set = 6, binding = 9) uniform sampler2D VSMshadowSampler;
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
    vec3 projCoords = fposLightSpace.xyz / fposLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    {
     float bias = 0.0;
     bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
     projCoords.z -= bias * 0.1;
    }
    float distToLight = texture(shadowSampler, projCoords.xy).r;

    if (projCoords.z <= distToLight)
        return 1.0;   
  
    else return 0.3;
} 


float chebyshevUpperBound( vec4 fragPosLightSpace) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 moments = texture(VSMshadowSampler, projCoords.xy).xy;  //E(x) D^2(x)
 
    if (projCoords.z <= moments.x)
        return 1.0;
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, 0.0002);
    float d = projCoords.z - moments.x;
    float p_max = variance / (variance + d * d);
    return p_max;
}



float CascadeCalculation(uint cascadeIndex)
{
	
    vec4 fragPosLightSpace  =  u.cascadeViewProjMat[cascadeIndex] * vec4(worldPos, 1.0);	
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
   
    projCoords = projCoords * 0.5 + 0.5;
    
    float shadow = 1.0;
	float bias = 0.005;

	
	float dist = texture(cascadeShadowSampler, vec3(projCoords.st, cascadeIndex)).r;
	if ( dist < projCoords.z - bias) {
			shadow = 0.3;
	}
	
	return shadow;
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

        shadow = CascadeCalculation(cascadeIndex);

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
            
            shadow = clamp( exp( -1000.0f * ( t- nearestOccluderDepth ) ), 0.0, 1.0 );  //e^(-c*(d-z))
            
        }
        else{
            shadow = ShadowCalculation(fragPosLightSpace, normal,lightDir);
        }
    }

    outColor = vec4( shadow* color.rgb,1.0 );
    
	
}


