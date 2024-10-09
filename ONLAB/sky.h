#pragma once

#include "vulkan/vulkan.h"
#include "helper.h"
#include "swapchain.h"
#include "descriptors.h"
#include<glm/glm.hpp>


namespace v {

	class Sky {
	private:
		Device& device;

		
		void prepareStorageImage(TextureResources& storageImage, DescriptorSetLayout& layout, DescriptorPool& pool, glm::vec2 texDim);
		void loadInputImage(DescriptorSetLayout& layout, DescriptorPool& pool);
		
		
	public:
		Sky(Device& device, DescriptorSetLayout& layout, DescriptorSetLayout& postComputelayout, DescriptorPool& pool);
		~Sky();

		TextureResources transmittanceLUT;
		TextureResources multiscatteringLUT;
		TextureResources skyviewLUT;

		TextureResources inputImage;

		std::vector<VkDescriptorSet> postComputeDescriptorSets;
		void createPostComputeDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& pool);

		void update();

	};


}