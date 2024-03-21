#pragma once


#include "vulkan/vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "renderinfo.h"

#include "pipelinemanager.h"

namespace v {


	class TerrainRenderSystem {
	private:

		struct pushConstantTesc {
			glm::vec2 viewport;
			float tessellationfactor;
		};

		Device& device;
		PipelineManager& pipelineManager;

		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipelineWireFrame;
		VkPipelineLayout pipelineLayout;

		std::array<int, 7> pushConstants;

		//VkRenderPass renderPass;

		//void createRenderPass();
		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		TerrainRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~TerrainRenderSystem();

		void renderTerrain(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo);
	};

}