#pragma once

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include "pipelinemanager.h"
#include "sky.h"
#include "gui.h"
#include "camera.h"

namespace v {

	class ComputeRenderSystem {
	private:

		Device& device;
		PipelineManager& pipelineManager;

		VkPipeline transmittancePipeline;
		VkPipelineLayout transmittancePipelineLayout;

		VkPipeline multiscatteringPipeline;
		VkPipelineLayout multiscatteringPipelineLayout;

		VkPipeline skyViewPipeline;
		VkPipelineLayout skyViewPipelineLayout;

		void createPipelineLayouts(std::vector<VkDescriptorSetLayout> setLayouts, VkPipelineLayout& pipelineLayout, bool skyview);
		void createPipeline(VkPipeline& pipeline, VkPipelineLayout pipelineLayout, const std::string& filename);

		

	public:
		ComputeRenderSystem(Device& device, PipelineManager& pipelineManager, std::vector<VkDescriptorSetLayout> setLayouts);
		~ComputeRenderSystem();

		void recordComputeCommandBuffers(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera);

		struct pushConstantCompute {
			glm::vec3 camera;
			float a;
			glm::vec3 sundir;
		};
	
	};

}

/*


#version 450


layout (local_size_x = 8, local_size_y = 4) in;

layout (set = 0, binding = 0, rgba8) uniform readonly image2D transmittanceLUT;
layout (set = 1, binding = 0, rgba8) uniform readonly image2D multiscatteringLUT;
layout (set = 2, binding = 0, rgba8) uniform image2D resultImage;

layout( push_constant ) uniform PushConstants {
   vec3 camera;
   float a;
   vec3 sundir;
} p;

vec2 multiscatteringTexDim =  vec2(32.0f, 32.0f);
vec2 transmittanceTexDim = vec2(256.0f, 64.0f);

const float PI = 3.1415;

const float atmosphereBottomR = 6360.0f;
const float atmosphereTopR = 6460.0f;


vec3 rayleighScatteringCoefficient = vec3(5.802, 13.558, 33.1) * 0.001;
float rayleighAbsorptioCoefficient = 0.0;

float mieScatteringCoefficient = 3.996  * 0.001;
float mieAbsorptionCoefficient = 4.4 * 0.001;

float ozoneScatteringCoefficient = 0.0;
vec3 ozoneAbsorptionCoefficient = vec3(0.650, 1.881, .085) * 0.001;


float safeSqrt(float x)
{
    return sqrt(max(0, x));
}

struct ScattValues{
    vec3 rayleighScattering;
    float mieScattering;
    vec3 extinction;
};
float getMiePhase(float cosTheta) {
    const float g = 0.8;
    const float scale = 3.0/(8.0*PI);

    float num = (1.0-g*g)*(1.0+cosTheta*cosTheta);
    float denom = (2.0+g*g)*pow((1.0 + g*g - 2.0*g*cosTheta), 1.5);

    return scale*num/denom;
}

float getRayleighPhase(float cosTheta) {
    const float k = 3.0/(16.0*PI);
    return k*(1.0+cosTheta*cosTheta);

}

ScattValues getScatteringValues(vec3 pos){
    ScattValues res;
    float height = (length(pos) - 6360.0f);

    float rayleighDensity = exp(-height / 8.0);     //surusegeloszlas
    float mieDensity = exp(-height / 1.2);

    res.rayleighScattering = rayleighScatteringCoefficient * rayleighDensity;
    res.mieScattering = mieScatteringCoefficient * mieDensity;
    float mieAbsorption = mieAbsorptionCoefficient * mieDensity;
    vec3 ozoneAbsorption = ozoneAbsorptionCoefficient * max(0.0, 1.0 - abs(height-25.0)/15.0);

    res.extinction = res.rayleighScattering + res.mieScattering + mieAbsorption + ozoneAbsorption;
    return res;
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

//https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/functions.glsl.html
ivec2 GetUvFromRMu(float r, float mu)
{
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
    float H = safeSqrt(atmosphereTopR * atmosphereTopR - atmosphereBottomR * atmosphereBottomR);
    // Distance to the horizon.
    float rho = safeSqrt(r * r - atmosphereBottomR * atmosphereBottomR);

    // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
    // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
    float discriminant = r * r * (mu * mu - 1.0) + atmosphereTopR * atmosphereTopR;
    float d = max(0.0, (-r * mu + safeSqrt(discriminant)));

    float d_min = atmosphereTopR - r;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;
    vec2 uv = vec2(x_mu, x_r);

    return ivec2(uv * transmittanceTexDim);
     //return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
     //         GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}


vec3 getMultipleScattering(vec3 worldPosition, float viewZenithCosAngle)
{
    vec2 uv = clamp(vec2(viewZenithCosAngle * 0.5 + 0.5, (length(worldPosition)-atmosphereBottomR) /(atmosphereTopR - atmosphereBottomR)), 0.0, 1.0);
    //
    ivec2 coords = ivec2(uv * multiscatteringTexDim);
    return imageLoad(multiscatteringLUT, coords).rgb;
}

vec3 integrateScatteredLuminance(vec3 worldPosition, vec3 worldDirection, vec3 sunDirection, int sampleCount)
{
    float cosTheta = dot( worldDirection,sunDirection);
    float miePhaseValue = getMiePhase(cosTheta);
    float rayleighPhaseValue = getRayleighPhase(-cosTheta);


    float groundDist = rayIntersectsSphere( worldPosition, worldDirection, atmosphereBottomR);
    float atmosphereDist = rayIntersectsSphere( worldPosition, worldDirection, atmosphereTopR);

    float integrationLength;
    integrationLength = atmosphereDist;
    if (groundDist > 0.0) {
        integrationLength = groundDist;
    }
    float dt = integrationLength / float(sampleCount);



    vec3 Tsum = vec3(1.0, 1.0, 1.0);
    vec3 lightsum = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < sampleCount; i++)
    {
        vec3 newPos = worldPosition + (i * dt) * worldDirection;

        ScattValues scattValues = getScatteringValues(newPos);
        vec3 scattering = scattValues.rayleighScattering + scattValues.mieScattering;
        vec3 extinction = max( scattValues.extinction , 0.0000001) ;

        vec3 transmittance = exp(-scattValues.extinction * dt);


        vec3 up = normalize(newPos);
        float r = length(newPos);
        float viewZenithCosAngle = dot(sunDirection, up);
        ivec2 uv = GetUvFromRMu(r, viewZenithCosAngle);

        vec3 transmittanceToSun = vec3(imageLoad(transmittanceLUT, uv).rgb);

        vec3 psi = getMultipleScattering(newPos, viewZenithCosAngle);

        //shadow?

        vec3 rayleighInScattering = scattValues.rayleighScattering*(rayleighPhaseValue*transmittanceToSun + psi);
        vec3 mieInScattering = scattValues.mieScattering*(miePhaseValue*transmittanceToSun + psi);
        vec3 inScattering = (rayleighInScattering + mieInScattering);

        vec3 sunLightInteg = (inScattering - inScattering * transmittance) / extinction;
        lightsum += sunLightInteg * Tsum;
        Tsum *= transmittance;
    }
    return lightsum;
}


void main()
{
        vec2 skyViewTexDim = vec2(192.0f, 128.0f);
        vec2 uv = vec2(gl_GlobalInvocationID.xy) / skyViewTexDim;


float height = p.camera.y * 0.1 + atmosphereBottomR;    //

vec3 sundir = normalize(p.sundir);
vec3 worldPos = vec3(0.0, height, 0.0);

float beta = asin(atmosphereBottomR / length(worldPos));
float fi = PI * 0.5 - beta;
float azimuthAngle = (uv.x * uv.x) * PI; //(uv.x - 0.5)*2.0*PI;

float l;
float x = 1.0 - 2.0 * uv.y;
if (uv.y < 0.5) {
    l = -x * x * PI * 0.5;
}
else {
    l = x * x * PI * 0.5;
}

//float height = length(worldPos);
vec3 up = worldPos / height;
float horizonAngle = asin(atmosphereBottomR / length(worldPos)) - 0.5 * PI;
float altitudeAngle = l - fi;

float cosAltitude = cos(altitudeAngle);

vec3 worldDirection = vec3(
    cos(azimuthAngle) * cos(altitudeAngle),
    sin(altitudeAngle),
    cos(altitudeAngle) * sin(azimuthAngle));

float sunZenithCosAngle = dot(normalize(worldPos), sundir);
vec3 localSunDirection = vec3(
    safeSqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle),
    sunZenithCosAngle,
    0.0);


vec3 Luminance = integrateScatteredLuminance(worldPos, worldDirection, localSunDirection, 30);
imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(Luminance, 1.0));

//vec3 multiscattering =  imageLoad(multiscatteringLUT, ivec2(gl_GlobalInvocationID.xy)).rgb;
//imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(multiscattering, 1.0));

}

*/