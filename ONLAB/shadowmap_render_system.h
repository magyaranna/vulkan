#pragma once

#include "device.h"
#include "pipeline.h"
#include "vertex.h"
#include "shadowmaps.h"
#include "renderinfo.h"
#include "timestamp_query.h"

#include <cassert>

namespace v {


	class ShadowmapRenderSystem {
	private:
		Device& device;
		

		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipeline_peterpanning;

		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:
		std::unique_ptr<TS_query> ts;

		ShadowmapRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass);
		~ShadowmapRenderSystem();

		
		void renderGameObjects(OffScreenRenderInfo renderinfo, DepthShadowMap& shadowMap);


		
	};
}