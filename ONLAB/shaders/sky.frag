#version 450

layout(location = 0) in vec2 uv;
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D computeResult;  
layout(set = 1, binding = 0) uniform GlobalUniformBufferObject { //
    mat4 view;
    mat4 proj;
} global;

layout (set = 2, binding = 0, rgba8) uniform readonly image2D transmittanceLUT;

layout( push_constant ) uniform PushConstants {
    vec3 camera;
	float a;
	vec3 sundir;
    float b;
} p;


const float PI = 3.1415;

const float atmosphereBottomR = 6360.0f;
const float atmosphereTopR = 6460.0f;

const vec2 transmittanceTexDim = vec2(256.0f, 64.0f);

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


vec3 jodieReinhardTonemap(vec3 c){
    // From: https://www.shadertoy.com/view/tdSXzD
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);
    return mix(c / (l + 1.0), tc, tc);
}

vec3 sunWithBloom(vec3 rayDir, vec3 sunDir) {
    const float sunSolidAngle = 0.53*PI/180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    float cosTheta = dot(rayDir, sunDir);
    if (cosTheta >= minSunCosTheta) return vec3(1.0);
    
    float offset = minSunCosTheta - cosTheta;
    float gaussianBloom = exp(-offset*50000.0)*0.5;
    float invBloom = 1.0/(0.02 + offset*300.0)*0.01;
    return vec3(gaussianBloom+invBloom);
}



//sphere: p^2 - r^2 = 0   origin = (0,0,0)   
 //ray: s + d * t   
float rayIntersectsSphere(vec3 s, vec3 d, float r)   
{
	float a = dot(d, d);
	float b = 2.0 * dot(s,d);
	float c = dot(s,s) - dot(r,r);
	float Disc = b * b - 4.0*a*c;
	if (Disc < 0.0 )
	{
		return -1.0;
	}
	float t1 = (-b - sqrt(max(0, Disc))) / (2.0*a);
	float t2 = (-b + sqrt(max(0, Disc))) / (2.0*a);
	if (t1 < 0.0 && t2 < 0.0)
	{
		return -1.0;
	}
	if (t1 < 0.0)
	{
		return max(0.0, t2);
	}
	else if (t2 < 0.0)
	{
		return max(0.0, t1);
	}
	return max(0.0, min(t1, t2));
}

float sun(vec3 viewDir, vec3 sunDir) {

    float cosTheta = dot(normalize(viewDir), normalize(sunDir));
    
    float sunRadius = 0.9995;
    float sunEdgeSoftness = 0.2;
    
    float sunDiskSize = sunRadius * (1.0 + 0.2 * (1.0 - abs(cosTheta)));
    float sunDisk = smoothstep(sunDiskSize, sunDiskSize + sunEdgeSoftness, cosTheta);
    
    float sunColorIntensity = 1.0 - abs(cosTheta);
    vec3 sunColor = mix(
        vec3(1.0, 0.9, 0.7),  
        vec3(1.0, 1.0, 1.0),  
        sunColorIntensity
    );
    
    return sunDisk * 50000.0 * sunColorIntensity;
}

ivec2 GetUvFromRMu(float r, float mu)
{
    float H = safeSqrt(atmosphereTopR * atmosphereTopR - atmosphereBottomR * atmosphereBottomR);
  	float rho = safeSqrt(r * r - atmosphereBottomR * atmosphereBottomR);
	
  	float discriminant = r * r * (mu * mu - 1.0) + atmosphereTopR * atmosphereTopR;
	float d = max(0.0, (-r * mu + safeSqrt(discriminant)));

	float d_min = atmosphereTopR - r;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;
    vec2 uv = vec2(x_mu, x_r);
   
	return ivec2(uv * transmittanceTexDim);  
   
}

void main() 
{
    vec3 camera = p.camera;
    vec3 sunDir = p.sundir;


    mat4 invViewProj = inverse(global.proj * global.view);
    vec3 clipSpace = vec3(uv*vec2(2.0) - vec2(1.0), 1.0f);  
 
    vec4 pixelHeight = invViewProj * vec4(clipSpace, 1.0);  

    vec3 viewDir = normalize(pixelHeight.xyz/pixelHeight.w); 
    vec3 worldPos = camera * 0.1 + vec3(0, atmosphereBottomR -0.0,0);  
    float viewHeight = length(worldPos);
    
    vec3 up = normalize(worldPos);
    
    float beta = asin(atmosphereBottomR / viewHeight);
    float fi = PI *0.5 - beta;
    

    float altitude = asin(dot(viewDir, up)) - fi;    //latitude
    float sky_v = 0.5 + 0.5*sign(altitude)*sqrt(abs(altitude)*2.0/PI);    //paper 5.3

    float azimuth = acos(dot(cross(up, viewDir),  cross(up, sunDir)));    // <rd, rs> = cOSTHETA
    float sky_u = azimuth / ( 2.0 *PI);   


    vec3 lum = vec3(texture(computeResult, vec2(sky_u, sky_v)).rgb);

    //if !groundIntersection
    //float sunLuminance = sun(viewDir, sunDir);    //TODO perspective torzitas!!
    //lum += vec3(sunLuminance);

    //outFragColor = vec4(lum, 1.0);  
    
	
	vec4 color = texture(computeResult, uv);
    //outFragColor = vec4(color);


    /**/

    vec3 sunLum = sunWithBloom(viewDir, sunDir);
   
   // sunLum = smoothstep(0.002, 1.0, sunLum);
    if (length(sunLum) > 0.0) {
        
            //hitPos = normalize(worldPos)*atmosphereBottomR;

            vec3 up = normalize(worldPos);
            float r = length(worldPos);    
            float mu = dot(sunDir, up); 
            ivec2 uv = GetUvFromRMu(r, mu);
  
            vec3 transmittanceToSun = vec3(imageLoad(transmittanceLUT, uv).rgb);
               
            sunLum *= transmittanceToSun * 20.0f;
        
    }
    lum += sunLum;
    
    lum *= 2.0;
    lum = pow(lum, vec3(1.0));
    lum = jodieReinhardTonemap(lum);
    lum *= 1.2;

    outFragColor = vec4(lum, 1.0);
	
}

