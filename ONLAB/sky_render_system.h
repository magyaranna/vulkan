#pragma once


#include "vulkan/vulkan.h"
#include "device.h"
#include "pipeline.h"
#include "pipelinemanager.h"
#include "sky.h"
#include "compute_render_system.h"

namespace v {

	class SkyRenderSystem {
		Device& device;
		PipelineManager& manager;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		SkyRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~SkyRenderSystem();

		void drawSky(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera);


		struct pushConstantSky {
			glm::vec3 camera;

		};

	};
}