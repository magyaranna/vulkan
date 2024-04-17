#pragma once


#include "vulkan/vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "renderinfo.h"

#include "pipelinemanager.h"

namespace v {


	class WaterRenderSystem {
	private:

		Device& device;
		PipelineManager& pipelineManager;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		WaterRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~WaterRenderSystem();

		void renderWater(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo, Camera& camera, float moveFactor);
	};

}