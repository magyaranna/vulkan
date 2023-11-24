#pragma once

#include "device.h"
#include "pipeline.h"
#include "vertex.h"
#include "shadowmaps.h"
#include "renderinfo.h"
#include "timestamp_query.h"

#include <cassert>


//https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps

namespace v {

	class VSM_RenderSystem {
	private:

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:
		std::unique_ptr<TS_query> ts;

		VSM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass);
		~VSM_RenderSystem();


		void renderGameObjects(OffScreenRenderInfo renderinfo, ColorShadowMap& shadowMap, VkDescriptorSet descriptorSetShadowmap);

	};
}