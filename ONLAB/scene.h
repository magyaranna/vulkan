#pragma once


#include "vulkan/vulkan.h"
#include "helper.h"
#include "swapchain.h"
#include "descriptors.h"


namespace v {

	class Scene {
	private:
		Device& device;
		
		void createDepthBufferResources(int width, int height, VkRenderPass& renderPass,
			DescriptorSetLayout& setLayout, DescriptorPool& pool);

	public:
		Scene(Device& device, VkExtent2D extent, VkRenderPass& renderPass, DescriptorSetLayout& setLayout, DescriptorPool& pool);
		~Scene();

		FramebufferResources depthBuffer;


		FramebufferResources& getDepthBuffer() { return depthBuffer; }
		VkFramebuffer getDepthFramebuffer() { return depthBuffer.framebuffer; }
	};

}