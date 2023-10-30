#pragma once


#include "vulkan/vulkan.h"
#include <vector>

#include "pipeline.h"
#include "device.h"
#include "light.h"
#include "terrain.h"
#include "gameobject.h"


//https://jankautz.com/publications/esm_gi08.pdf

namespace v {

	struct ESMShadowmap {

		VkImage colorImage;
		VkDeviceMemory colorMem;
		VkImageView colorView;

		VkFramebuffer frameBuffer;
		VkSampler sampler;

		std::vector<VkDescriptorSet> descriptorSets;
	};
#define SHADOWMAP_DIM 3000

	class ESM_RenderSystem {
	private:
		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline();

		VkRenderPass renderPass;
		ESMShadowmap shadowMapESM;



		void createShadowRenderPass();
		void createShadowmapResources();

		void createShadowmapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);


	public:

		ESM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
		~ESM_RenderSystem();

		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int,
			std::unique_ptr<GameObject>>&gameobjects, std::unique_ptr<Terrain> const& terrain, VkDescriptorSet shadowmap);


		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return shadowMapESM.descriptorSets[i];
		}


	};

}