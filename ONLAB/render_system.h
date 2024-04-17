#pragma once

#include "device.h"
#include "pipeline.h"

#include "renderinfo.h"
#include "pipelinemanager.h"


namespace v {

	class RenderSystem {
	private:

		Device& device;
		PipelineManager& pipelineManager;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;


		//std::unique_ptr<Gui> gui; 

		std::array<int, 7> pushConstants;


		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		RenderSystem(Device& device, PipelineManager& pipelineManager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~RenderSystem();

		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderinfo, Camera& camera, glm::vec4 clipPlane = glm::vec4(0.0, -1.0, 0.0, 100000.0));

		//std::array<int, 6> getPushConstants() { return pushConstants; }
		void setPushConstants(int i, int value) { pushConstants[i] = value; }
	};
}