#version 450


layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} global;

layout(set = 3, binding = 1) uniform UniformBufferObject {
    mat4 model;
} obj;

layout( push_constant ) uniform pushConstants {
  layout(offset = 32) vec4 clipPlane; //normal + dist
} p;


layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;


layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 uv;
layout(location = 3) out vec4 worldPos;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out mat3 TBN;

#extension GL_EXT_clip_cull_distance: enable


void main() {

    fragPos = inPos;
    fragNormal = (inverse(transpose(global.view * obj.model)) * vec4(inNormal, 1.0)).xyz;
    uv = inTexCoord;
    worldPos = (obj.model * vec4(inPos, 1.0));  
    viewPos = (global.view * obj.model * vec4(inPos, 1.0)).xyz;  

    vec3 T = normalize(vec3(obj.model * vec4(inTangent,   0.0)));
    vec3 B = normalize(vec3(obj.model * vec4(inBitangent, 0.0)));
    vec3 N = normalize(vec3(obj.model * vec4(inNormal,    0.0)));
    TBN = mat3(T, B, N);

    vec4 position = global.proj * global.view * obj.model * vec4(inPos, 1.0);

    vec3 planeNormal = normalize(vec3(0.0, 1.0, 0.0)); 
    vec3 planePoint = vec3(0.0,50.0, 0.0); 
    float dist = dot(planeNormal, worldPos.xyz - planePoint);



    gl_ClipDistance[0] = dot(worldPos ,p.clipPlane);
    gl_Position = position;

}