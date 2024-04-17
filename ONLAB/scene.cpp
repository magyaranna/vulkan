
#include "scene.h"

namespace v {


	Scene::Scene(Device& device, VkExtent2D extent, VkRenderPass& renderPass, DescriptorSetLayout& setLayout, DescriptorPool& pool) : device(device){
		createDepthBufferResources(extent.width, extent.height, renderPass, setLayout, pool);
	}

	Scene::~Scene() {
		vkDestroyImageView(device.getLogicalDevice(), depthBuffer.res.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), depthBuffer.res.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), depthBuffer.res.mem, nullptr);
		vkDestroyFramebuffer(device.getLogicalDevice(), depthBuffer.framebuffer, nullptr);
		vkDestroySampler(device.getLogicalDevice(), depthBuffer.res.sampler, nullptr);
	}

	void Scene::createDepthBufferResources(int width, int height, VkRenderPass& renderPass, DescriptorSetLayout& setLayout, DescriptorPool& pool ) {
		depthBuffer.res.width = width;
		depthBuffer.res.height = height;

		VkFormat depthFormat = Helper::findDepthFormat(device);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = depthBuffer.res.width;
		imageInfo.extent.height = depthBuffer.res.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = depthFormat;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &depthBuffer.res.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), depthBuffer.res.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &depthBuffer.res.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), depthBuffer.res.image, depthBuffer.res.mem, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = depthBuffer.res.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &depthBuffer.res.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &depthBuffer.res.view;
		framebufferInfo.width = depthBuffer.res.width;
		framebufferInfo.height = depthBuffer.res.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &depthBuffer.framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}


		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;              
		samplerInfo.compareOp = VK_COMPARE_OP_LESS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 200.0f;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &depthBuffer.res.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		depthBuffer.res.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorImageInfo imageInfo{};     
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = depthBuffer.res.view;
			imageInfo.sampler = depthBuffer.res.sampler;

			DescriptorWriter(setLayout, pool)
				.createDescriptorWriter(0, &imageInfo)
				.build(depthBuffer.res.descriptorSets[i]);
		}
	}
}