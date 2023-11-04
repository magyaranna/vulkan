#pragma once

#include "device.h"
#include "pipeline.h"
#include "vertex.h"
#include "shadowmaps.h"
#include "renderinfo.h"

#include <cassert>

namespace v {


	class OffScreenRenderSystem {
	private:
		Device& device;


		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipeline_peterpanning;

		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		OffScreenRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass);
		~OffScreenRenderSystem();


		void renderGameObjects(OffScreenRenderInfo renderinfo, DepthShadowMap& shadowMap);


		
	};
}