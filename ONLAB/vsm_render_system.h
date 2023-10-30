#pragma once

#include "device.h"
#include "pipeline.h"
#include "gameobject.h"
#include "light.h"
#include "terrain_render_system.h"
#include "vertex.h"


#include <cassert>


//https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps

namespace v {

	struct VSMShadowmap {

		uint32_t mipLevels;

		VkImage MSAAcolorImage;
		VkDeviceMemory MSAAcolorMem;
		VkImageView MSAAcolorView;

		VkImage colorImage;
		VkDeviceMemory colorMem;
		VkImageView colorView;

		VkFramebuffer frameBuffer;
		VkSampler sampler;

		std::vector<VkDescriptorSet> descriptorSets;
	};

#define SHADOWMAP_DIM 4000

	class VSM_RenderSystem {
	private:

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline();

		VkRenderPass renderPass;
		VSMShadowmap shadowMapVSM;



		void createShadowRenderPass();
		void createShadowmapResources();

		void createShadowmapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);


	public:

		VSM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowLayout, VkDescriptorPool pool);
		~VSM_RenderSystem();


		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain);



		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return shadowMapVSM.descriptorSets[i];
		}

		VSMShadowmap getShadowMapVSM() { return shadowMapVSM; };
	};
}