
#include "sky.h"
#include <stb_image.h>

namespace v {

	Sky::Sky(Device& device, DescriptorSetLayout& layout, DescriptorSetLayout& posComputeLayout, DescriptorPool& pool) : device(device) {
		loadInputImage(layout, pool);
		prepareStorageImage(transmittanceLUT, layout, pool, glm::vec2(256, 64));
		prepareStorageImage(multiscatteringLUT, layout, pool, glm::vec2(32, 32)); 
		prepareStorageImage(skyviewLUT, layout, pool, glm::vec2(192, 128)); 
		createPostComputeDescriptorSets(posComputeLayout, pool);
	}
		

		

	Sky::~Sky() {
		vkDestroyImage(device.getLogicalDevice(), transmittanceLUT.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), transmittanceLUT.mem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), transmittanceLUT.view, nullptr);
		vkDestroySampler(device.getLogicalDevice(), transmittanceLUT.sampler, nullptr);

		vkDestroyImage(device.getLogicalDevice(), multiscatteringLUT.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), multiscatteringLUT.mem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), multiscatteringLUT.view, nullptr);
		vkDestroySampler(device.getLogicalDevice(), multiscatteringLUT.sampler, nullptr);

		vkDestroyImage(device.getLogicalDevice(), skyviewLUT.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), skyviewLUT.mem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), skyviewLUT.view, nullptr);
		vkDestroySampler(device.getLogicalDevice(), skyviewLUT.sampler, nullptr);

		vkDestroyImage(device.getLogicalDevice(), inputImage.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), inputImage.mem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), inputImage.view, nullptr);
		vkDestroySampler(device.getLogicalDevice(), inputImage.sampler, nullptr);
	}

	void Sky::prepareStorageImage(TextureResources& storageImage, DescriptorSetLayout& layout, DescriptorPool& pool, glm::vec2 texDim){ 

		const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		storageImage.width = texDim.x;
		storageImage.height = texDim.y;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = storageImage.width;
		imageInfo.extent.height = storageImage.height;
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		std::vector<uint32_t> queueFamilyIndices;
		if (device.indices.graphicsFamily != device.indices.computeFamily) {
			queueFamilyIndices = {
				device.indices.graphicsFamily.value(),     //???
				device.indices.computeFamily.value()
			};
			imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			imageInfo.queueFamilyIndexCount = 2;
			imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}

		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &storageImage.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), storageImage.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &storageImage.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}
		vkBindImageMemory(device.getLogicalDevice(), storageImage.image, storageImage.mem, 0);
		
		Helper::transitionImageLayout(device, storageImage.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);

		storageImage.view = Helper::createImageView(device, storageImage.image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &storageImage.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
		

		storageImage.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < storageImage.descriptorSets.size(); i++) {
			VkDescriptorImageInfo textureImageInfo{};     
			textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			textureImageInfo.imageView = storageImage.view;
			textureImageInfo.sampler = storageImage.sampler;

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &textureImageInfo)
				.build(storageImage.descriptorSets[i]);
		}

	}

	void Sky::createPostComputeDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& pool) {

		postComputeDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < postComputeDescriptorSets.size(); i++) {
			
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.imageView = skyviewLUT.view;
			imageInfo.sampler = skyviewLUT.sampler;

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &imageInfo)
				.build(postComputeDescriptorSets[i]);
		}
	}
	
	void Sky::loadInputImage(DescriptorSetLayout& layout, DescriptorPool& pool) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		inputImage.height = texHeight;
		inputImage.width = texWidth;


		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Helper::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

		stbi_image_free(pixels);

		Helper::createImage(device, texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, inputImage.image, inputImage.mem);

		Helper::transitionImageLayout(device, inputImage.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		Helper::copyBufferToImage(device, stagingBuffer, inputImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

		Helper::transitionImageLayout(device, inputImage.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 1, 1);

		vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);


		inputImage.view = Helper::createImageView(device, inputImage.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &inputImage.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		inputImage.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < inputImage.descriptorSets.size(); i++) {
			VkDescriptorImageInfo textureImageInfo{};
			textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			textureImageInfo.imageView = inputImage.view;
			textureImageInfo.sampler = inputImage.sampler;

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &textureImageInfo)
				.build(inputImage.descriptorSets[i]);
		}
	}

	
}