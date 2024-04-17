#version 450

//layout(set = 5, binding = 0) uniform sampler2D shadowSampler;
//layout(set = 6, binding = 0) uniform sampler2DArray  cascadeShadowSampler;
/*layout (set = 7, binding = 5) uniform UniformLightSpace {
	mat4 cascadeViewProjMat[4];
    vec4 cascadeSplits;
} u;*/
//layout(set = 8, binding = 0) uniform sampler2D VSMshadowSampler;
//layout(set = 9, binding = 0) uniform sampler2D ESMshadowSampler;


layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 0) uniform sampler2D normalMap;


layout(set = 4, binding = 0) uniform UniformBufferLightVP {
    mat4 view;
    mat4 proj;
    vec3 pos;
    vec3 dir;
} light;




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
layout(location = 3) in vec4 worldPos;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 outColor;


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
    vec3 reflectDir = reflect(-lightDir, normal); 
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * specularStrength *lightColor;


    /*COLOR*/
    vec4 color = vec4(1.0);
    if(p.displayNormalmap == 1){ 
         color = vec4(normal, 1.0);
        
    }else{
         color = texture(texSampler, uv) * vec4((ambient +  diffuse), 1.0);       
    }

    float shadow = 0;
    vec4 fragPosLightSpace = light.proj * light.view * vec4(worldPos.xyz,1.0);

    float alpha = texture(texSampler, uv).a;
	if (alpha < 0.5) {
	    	discard;
	}

    outColor = vec4(/*shadow **/ color.rgb , 1.0 );
    
}

