#version 450


//VP  material  M  light

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} global;

layout(set = 2, binding = 1) uniform UniformBufferObject {
    mat4 model;
} obj;


layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 viewPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 worldPos;
layout(location = 5) out mat3 TBN;

layout(location = 9) out vec3 pos;


void main() {
    gl_Position = global.proj * global.view * obj.model * vec4(inPos, 1.0);
    
    worldPos = (obj.model * vec4(inPos, 1.0)).xyz;  
    viewPos = (global.view * obj.model * vec4(inPos, 1.0)).xyz;  
    

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = (inverse(transpose(global.view * obj.model)) * vec4(inNormal, 1.0)).xyz;

   
    vec3 T = normalize(vec3(obj.model * vec4(inTangent,   0.0)));
    vec3 B = normalize(vec3(obj.model * vec4(inBitangent, 0.0)));
    vec3 N = normalize(vec3(obj.model * vec4(inNormal,    0.0)));
    TBN = mat3(T, B, N);

    pos = inPos;

   }