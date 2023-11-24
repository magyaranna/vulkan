#pragma once


#include "vulkan/vulkan.h"
#include <vector>

#include "pipeline.h"
#include "device.h"

#include "shadowmaps.h"
#include "renderinfo.h"
#include "timestamp_query.h"

//https://jankautz.com/publications/esm_gi08.pdf

namespace v {


	class ESM_RenderSystem {
	private:
		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:
		std::unique_ptr<TS_query> ts;

		ESM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass);
		~ESM_RenderSystem();

		void renderGameObjects(OffScreenRenderInfo renderinfo, ColorShadowMap& shadowMap, VkDescriptorSet shadowmap);

	};

}