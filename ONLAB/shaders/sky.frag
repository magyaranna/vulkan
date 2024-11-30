#version 450

layout(location = 0) in vec2 uv;
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D computeResult;  
layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { //
    mat4 view;
    mat4 proj;
} global;

layout( push_constant ) uniform PushConstants {
    vec3 camera;
	float a;
	vec3 sundir;
} p;


const float PI = 3.1415;

const float atmosphereBottomR = 6360.0f;
const float atmosphereTopR = 6460.0f;

float safeSqrt(float x)
{
    return sqrt(max(0, x));
}


float Sun(vec3 viewDir, vec3 sunDir) {
    float cosTheta = dot(viewDir, sunDir);
    float sunRadius = 0.9995;
    float sunDisk = smoothstep(sunRadius, sunRadius + 0.2, cosTheta);
    return sunDisk * 50000.0;
}

void main() 
{
    vec3 camera = p.camera;
    vec3 sunDir = p.sundir;


    mat4 invViewProj = inverse(global.proj * global.view);
    vec3 clipSpace = vec3(uv*vec2(2.0) - vec2(1.0), 1.0f);  
 
    vec4 pixelHeight = invViewProj * vec4(clipSpace, 1.0);  

    vec3 viewDir = normalize(pixelHeight.xyz/pixelHeight.w - camera); 
    vec3 worldPos = camera * 0.1 + vec3(0, atmosphereBottomR,0);  
    float viewHeight = length(worldPos);
    
    vec3 up = normalize(worldPos);
    
    float beta = asin(atmosphereBottomR / viewHeight);
    float fi = PI *0.5 - beta;
    

    float altitude = asin(dot(viewDir, up)) - fi;    //latitude
    float sky_v = 0.5 + 0.5*sign(altitude)*sqrt(abs(altitude)*2.0/PI);    //paper 5.3

    float azimuth = acos(dot(cross(up, viewDir),  cross(up, sunDir)));    // <rd, rs> = cOSTHETA
    float sky_u = azimuth / (2.0 * PI);   


    vec3 lum = vec3(texture(computeResult, vec2(sky_u, sky_v)).rgb);

    //if !groundIntersection
    float sunLum = Sun(viewDir, sunDir);    //TODO perspective torzitas!!
    lum += vec3(sunLum);

    outFragColor = vec4(lum, 1.0);  
    
	
	//vec4 color = texture(computeResult, vec2(uv.x, 1.0-uv.y));
	//outFragColor = vec4(color);
	
}

