#pragma once

#include "device.h"
#include "pipeline.h"
#include "gameobject.h"
#include "light.h"
#include "terrain_render_system.h"
#include "vertex.h"


#include <cassert>

namespace v {

	struct SimpleShadowmap {

		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFramebuffer frameBuffer;
		VkSampler sampler;

		std::vector<VkDescriptorSet> descriptorSets;
	};
#define SHADOWMAP_DIM 4000

	class OffScreenRenderSystem {
	private:

		Device& device;

		VkRenderPass renderPass;

		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipeline_peterpanning;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline();


		SimpleShadowmap shadowMap;

		void createShadowRenderPasses();
		void createShadowmapResources();

		void createShadowmapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);



	public:

		OffScreenRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowLayout, VkDescriptorPool pool);
		~OffScreenRenderSystem();


		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, bool peterpanning, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain);


		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return shadowMap.descriptorSets[i];
		}
	};
}