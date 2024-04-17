#pragma once


#include "vulkan/vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "renderinfo.h"

#include "pipelinemanager.h"

namespace v {


	class TerrainRenderSystem {
	private:
	
		Device& device;
		PipelineManager& pipelineManager;

		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipelineWireFrame;
		VkPipelineLayout pipelineLayout;

		
		//VkRenderPass renderPass;

		//void createRenderPass();
		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		struct pushConstantTesc {
			glm::vec2 viewport;
			float tessellationfactor;
		};

		struct pushConstantTese {
			float displacementFactor;
			glm::vec4 clipPlane;
		};
		std::array<int, 7> pushConstants;


		TerrainRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~TerrainRenderSystem();

		void renderTerrain(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo, Camera& camera, glm::vec4 clipPlane = glm::vec4(0.0, -1.0, 0.0, 100000.0));

		VkPipelineLayout& getPipelineLayout() {
			return pipelineLayout;
		}

	};

}