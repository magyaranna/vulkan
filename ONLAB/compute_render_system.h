#pragma once

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include "pipelinemanager.h"
#include "sky.h"
#include "gui.h"
#include "camera.h"

namespace v {

	class ComputeRenderSystem {
	private:

		Device& device;
		PipelineManager& pipelineManager;

		VkPipeline transmittancePipeline;
		VkPipelineLayout transmittancePipelineLayout;

		VkPipeline multiscatteringPipeline;
		VkPipelineLayout multiscatteringPipelineLayout;

		VkPipeline skyViewPipeline;
		VkPipelineLayout skyViewPipelineLayout;

		void createPipelineLayouts(std::vector<VkDescriptorSetLayout> setLayouts, VkPipelineLayout& pipelineLayout);
		void createPipeline(VkPipeline& pipeline, VkPipelineLayout pipelineLayout, const std::string& filename);

		

	public:
		ComputeRenderSystem(Device& device, PipelineManager& pipelineManager, std::vector<VkDescriptorSetLayout> setLayouts);
		~ComputeRenderSystem();

		void recordComputeCommandBuffers(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera);

	
	};

}
