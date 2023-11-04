#pragma once


#include "vulkan/vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "renderinfo.h"

namespace v {


	class TerrainRenderSystem {
	private:

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		std::array<int, 7> pushConstants;

		//VkRenderPass renderPass;

		//void createRenderPass();
		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		TerrainRenderSystem(Device& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~TerrainRenderSystem();

		void renderTerrain(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo);



	};

}