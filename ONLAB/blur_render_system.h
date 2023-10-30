#pragma once


#pragma once

#include "device.h"
#include "pipeline.h"

#include <cassert>

namespace v {

#define SHADOWMAP_DIM 4000


	struct FrameBufferResources {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFramebuffer frameBuffer;
	};


	class BlurSystem {
	private:

		Device& device;


		//VkImage image;
		//VkDeviceMemory mem;
		//VkImageView view;
		//VkFramebuffer frameBuffer;
		VkSampler sampler;


		VkPipelineLayout pipelineLayout;

		std::unique_ptr<Pipeline> pipelineBlurHorz;
		std::unique_ptr<Pipeline> pipelineBlurVert;


		VkRenderPass renderPass;


		std::vector<VkDescriptorSet> descriptorSetsBlurHorz;
		std::vector<VkDescriptorSet> descriptorSets;

		FrameBufferResources horizontal_fb;
		FrameBufferResources vertical_fb;


		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline();

		std::array<int, 1> pushConstants;

		void createRenderPass();
		void createFrameBufferResources(FrameBufferResources& fb);
		void createSampler();

		void createDescriptorSets_BluredH(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
		void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);



	public:

		BlurSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout VSMLayout, VkDescriptorSetLayout layout, VkDescriptorPool pool);
		~BlurSystem();

		void render(VkCommandBuffer& cmd, VkDescriptorSet image, int index);

		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return descriptorSets[i];
		}

	};
}