#pragma once


#include "vulkan/vulkan.h"
#include <vector>

#include "pipeline.h"
#include "device.h"

#include "renderinfo.h"
#include "pipelinemanager.h"

namespace v {


	class Normalmap_RenderSystem {
	private:
		Device& device;
		PipelineManager& pipelineManager;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);
		void createCommandBuffers();
		VkCommandBuffer commandBuffer;
	public:

		Normalmap_RenderSystem(Device& device, PipelineManager& pipelineManager, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass);
		~Normalmap_RenderSystem();

		void make_normalmap(Terrain& terrain, VkRenderPass renderPass);

	};

}