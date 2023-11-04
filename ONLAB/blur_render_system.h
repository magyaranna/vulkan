#pragma once


#pragma once

#include "device.h"
#include "pipeline.h"

#include "shadowmaps.h"

#include <cassert>

namespace v {


	class BlurSystem {
	private:

		Device& device;

		

		std::unique_ptr<ColorShadowMap> tempShadowMap = nullptr;
		std::unique_ptr<ColorCascadeShadowMap> tempCascadeShadowMap = nullptr;


		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Pipeline> pipeline;

		VkPipelineLayout pipelineLayoutCascade;
		std::unique_ptr<Pipeline> pipelineCascade;



		void createPipelineLayouts(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipelines(VkRenderPass renderPass);

		std::array<int, 1> pushConstants;
		std::array<int, 2> cascadePushConstants; //blurdir cascadeidx


	public:

		BlurSystem(Device& device, int binding, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout blurlayout, VkDescriptorPool pool, VkRenderPass renderPass);
		~BlurSystem();

		void render(VkCommandBuffer& cmd, int index, ColorShadowMap& shadowMap, VkDescriptorSet image, VkRenderPass renderPass);
		void render(VkCommandBuffer& cmd, int index, ColorCascadeShadowMap& shadowMap, VkDescriptorSet image, VkRenderPass renderPass);

		

	};
}