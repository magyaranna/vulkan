#pragma once

#include "device.h"
#include "pipeline.h"

#include "renderinfo.h"


namespace v {

	class RenderSystem {
	private:

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;


		//std::unique_ptr<Gui> gui; 

		std::array<int, 5> pushConstants;


		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		RenderSystem(Device& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~RenderSystem();

		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderinfo);

		std::array<int, 5> getPushConstants() { return pushConstants; }
		void setPushConstants(int i, int value) { pushConstants[i] = value; }
	};
}