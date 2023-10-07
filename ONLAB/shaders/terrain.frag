#version 450

layout(set = 3, binding = 4) uniform sampler2DShadow shadowSampler;
layout(set = 4, binding = 7) uniform sampler2DArray  cascadeShadowSampler;
layout(set = 6, binding = 9) uniform sampler2D VSMshadowSampler;

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


float chebyshevUpperBound(vec4 fposLightSpace, float distance) {
    vec2 moments = texture(VSMshadowSampler, fposLightSpace.xy).rg;  //E(x) D^2(x)
 
    if (distance <= moments.x)
        return 1.0;
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, 0.00002);
    float d = distance - moments.x;
    float p_max = variance / (variance + d * d);
    return p_max;
}


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
}  




float CascadeCalculation(uint cascadeIndex)
{
	
    //cascadeIndex = 1;
    vec4 shadowCoord = (biasMat * u.cascadeViewProjMat[cascadeIndex]) * vec4(worldPos, 1.0);	//worldPos

	float shadow = 1.0;
	float bias = 0.005;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(cascadeShadowSampler, vec3(shadowCoord.st, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 0.3;
		}
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

    uint cascadeIndex = 0;
	for(uint i = 0; i < 3; i++) {
		 if(viewPos.z < u.cascadeSplits[i]*10) {	   /////
			 cascadeIndex = i + 1;
		 }
	}

	if(p.cascade == 1){
        

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
            float distance = fragPosLightSpace.z / fragPosLightSpace.w;
            shadow = chebyshevUpperBound(fragPosLightSpace, distance);
        }
         else if(p.esm == 1){
            float distance = fragPosLightSpace.z / fragPosLightSpace.w;    //d

            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords.xy = projCoords.xy * 0.5 + 0.5;
            float nearestOccluderDepth = texture(shadowSampler, projCoords.xyz).r;
            
            shadow = clamp( exp( 8.0f * ( nearestOccluderDepth - distance ) ), 0.0, 1.0 );  //e^(-c*(d-z))
            
        }
        else{
            shadow = ShadowCalculation(fragPosLightSpace, normal,lightDir);
        }
    }

    outColor = vec4( shadow* color.rgb,1.0 );
    
	
}


