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
		bool vsm;

		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFramebuffer frameBuffer;
		VkSampler sampler;

		VkRenderPass renderPass;

		std::vector<VkDescriptorSet> descriptorSets;
		std::unique_ptr<Pipeline> pipeline;
	};
#define SHADOWMAP_DIM 2000

	class OffScreenRenderSystem {
	private:

		Device& device;


		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(SimpleShadowmap& sm, bool vsm);

		std::array<int, 1> pushConstants;

		/*shadowmap*/
		SimpleShadowmap shadowMap;
		SimpleShadowmap shadowMapVSM;



		void createShadowRenderPasses();
		void createShadowmapResources(SimpleShadowmap& sm);

		void createShadowmapDescriptorSets(SimpleShadowmap& sm, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);



	public:

		OffScreenRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowLayout, VkDescriptorPool pool);
		~OffScreenRenderSystem();


		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, bool vsm, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain);


		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return shadowMap.descriptorSets[i];
		}
	};
}