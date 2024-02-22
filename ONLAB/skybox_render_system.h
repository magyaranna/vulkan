#pragma once


#include "device.h"
#include "pipeline.h"

#include "skybox.h"
#include "camera.h"


namespace v {

	class SkyboxRenderSystem {
	private:

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		SkyboxRenderSystem(Device& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~SkyboxRenderSystem();

		void drawSkybox(VkCommandBuffer& cmd, int currentFrame, SkyBox& skybox, Camera& camera);

	
	};

}
