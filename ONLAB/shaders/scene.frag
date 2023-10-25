#version 450

layout(set = 4, binding = 4) uniform sampler2DShadow shadowSampler;
layout(set = 5, binding = 7) uniform sampler2DArray  cascadeShadowSampler;
layout(set = 7, binding = 9) uniform sampler2D VSMshadowSampler;

layout(set = 1, binding = 5) uniform sampler2D texSampler;
layout(set = 1, binding = 6) uniform sampler2D normalMap;


layout(set = 3, binding = 2) uniform UniformBufferLightVP {
    mat4 view;
    mat4 proj;
} light;

layout(set = 3, binding = 3) uniform UniformBufferLight {
    vec4 pos;
    vec3 dir;
} lbo;

layout (set = 6, binding = 8) uniform UniformLightSpace {
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
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 worldPos;
layout(location = 5) in mat3 TBN;

layout(location = 9) in vec3 inPos;


layout(location = 0) out vec4 outColor;


void main() {

    vec3 normal = texture(normalMap, fragTexCoord).rgb;   
    normal = normalize(normal * 2.0 - 1.0); 
    normal = normalize(TBN * normal);

    vec3 lightDir = normalize(lbo.dir);
    vec3 lightColor = vec3(1.0f,1.0f,1.0f);
    float ambientStrength = 0.5;  //0.2
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = max(dot(normal,-lightDir),0.0) * lightColor;

    vec4 fragPosLightSpace = light.proj * light.view * vec4(worldPos,1.0);

    /*COLOR*/
    vec4 color = vec4(1.0);
    if(p.displayNormalmap == 1){
        color = vec4(normal, 1.0);
    }else{
        color = texture(texSampler,fragTexCoord) * vec4((ambient +  diffuse),1.0);
    }
    

    /*Shadow*/
    float shadow = 1.0;
   
    outColor = vec4(shadow * color.rgb,  color.a );
    
}




