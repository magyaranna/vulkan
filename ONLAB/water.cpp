

#include "water.h"
#include <stb_image.h>


namespace v {

	Water::Water(Device& device, VkRenderPass renderPass, DescriptorSetLayout& layout, DescriptorPool& pool) : device(device) {
		createVertexBuffer();
		createIndexBuffer();
		createRenderPass();
		createOffscreenFrameBuffer(reflection, renderPass, layout, pool);
		createOffscreenFrameBuffer(refraction, renderPass, layout, pool);

		createTextureDescriptorSet(layout, pool);
	}

	Water::~Water() {
		vkDestroyImageView(device.getLogicalDevice(), reflection.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), reflection.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), reflection.mem, nullptr);

		vkDestroyImageView(device.getLogicalDevice(), reflection.colorView, nullptr);
		vkDestroyImage(device.getLogicalDevice(), reflection.colorImage, nullptr);
		vkFreeMemory(device.getLogicalDevice(), reflection.colorMem, nullptr);

		vkDestroyImageView(device.getLogicalDevice(), reflection.depthView, nullptr);
		vkDestroyImage(device.getLogicalDevice(), reflection.depthImage, nullptr);
		vkFreeMemory(device.getLogicalDevice(), reflection.depthMem, nullptr);

		vkDestroyFramebuffer(device.getLogicalDevice(), reflection.frameBuffer, nullptr);
		vkDestroySampler(device.getLogicalDevice(), reflection.sampler, nullptr);
		//
		vkDestroyImageView(device.getLogicalDevice(), refraction.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), refraction.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), refraction.mem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), refraction.colorView, nullptr);
		vkDestroyImage(device.getLogicalDevice(), refraction.colorImage, nullptr);
		vkFreeMemory(device.getLogicalDevice(), refraction.colorMem, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), refraction.depthView, nullptr);
		vkDestroyImage(device.getLogicalDevice(), refraction.depthImage, nullptr);
		vkFreeMemory(device.getLogicalDevice(), refraction.depthMem, nullptr);

		vkDestroyFramebuffer(device.getLogicalDevice(), refraction.frameBuffer, nullptr);
		vkDestroySampler(device.getLogicalDevice(), refraction.sampler, nullptr);

		vkDestroyRenderPass(device.getLogicalDevice(), renderPass, nullptr);


		vkDestroySampler(device.getLogicalDevice(), dudvTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), dudvTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), dudvTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), dudvTexture.mem, nullptr);

	}

	void Water::createTextureDescriptorSet(DescriptorSetLayout& layout, DescriptorPool& pool) {

		std::string texture_path = "textures/dudvmap.png";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		size_t imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}


		Buffer stagingBuffer{ device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		stagingBuffer.map(imageSize);
		stagingBuffer.writeToBuffer((void*)pixels);
		stagingBuffer.unmap();
		stbi_image_free(pixels);

		Helper::createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dudvTexture.image, dudvTexture.mem);

		Helper::transitionImageLayout(device, dudvTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
		Helper::copyBufferToImage(device, stagingBuffer.getBuffer(), dudvTexture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		Helper::generateMipmaps(device, dudvTexture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

		dudvTexture.view = Helper::createImageView(device, dudvTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


		/*sampler*/
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
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &dudvTexture.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}


		/*descriptorsets*/
		dudvTexture.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkDescriptorImageInfo textureImageInfo{};       //texture
		textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		textureImageInfo.imageView = dudvTexture.view;
		textureImageInfo.sampler = dudvTexture.sampler;

		for (size_t i = 0; i < dudvTexture.descriptorSets.size(); i++) {
			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &textureImageInfo)
				.build(dudvTexture.descriptorSets[i]);
		}
	}

	void Water::createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
		colorAttachment.samples = device.getMSAASampleCountFlag();
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = Helper::findDepthFormat(device);
		depthAttachment.samples = device.getMSAASampleCountFlag();;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = VK_FORMAT_B8G8R8A8_SRGB;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}



	void Water::createOffscreenFrameBuffer(OffScreenFrameBuffer& fb, VkRenderPass renderPass, DescriptorSetLayout& layout, DescriptorPool& pool) {

		Helper::createImage(device, 800, 600, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fb.image, fb.mem);
		fb.view = Helper::createImageView(device, fb.image, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);


		Helper::createImage(device, 800, 600, 1, device.getMSAASampleCountFlag(), VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fb.colorImage, fb.colorMem);
		fb.colorView = Helper::createImageView(device, fb.colorImage, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkFormat depthFormat = Helper::findDepthFormat(device);
		Helper::createImage(device, 800, 600, 1, device.getMSAASampleCountFlag(), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fb.depthImage, fb.depthMem);
		fb.depthView = Helper::createImageView(device, fb.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);


		std::array<VkImageView, 3> attachments = {
			fb.colorView,
			fb.depthView,
			fb.view
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = 800;
		framebufferInfo.height = 600;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &fb.frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}


		/*sampler*/
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
		samplerInfo.compareEnable = VK_TRUE;              
		samplerInfo.compareOp = VK_COMPARE_OP_LESS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 200.0f;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &fb.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		/*descriptorset*/
		fb.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < fb.descriptorSets.size(); i++) {
			VkDescriptorImageInfo textureImageInfo{};       //texture
			textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureImageInfo.imageView = fb.view;
			textureImageInfo.sampler = fb.sampler;

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &textureImageInfo)
				.build(fb.descriptorSets[i]);
		}
	}
	

	void Water::createVertexBuffer() {

		
		Vertex vertex{};
		vertex.pos = glm::vec3(-1.0f * scale, height, -1.0f * scale);
		vertex.texCoord = glm::vec2(0.0, 0.0);
		vertices.push_back(vertex);

		vertex.pos = glm::vec3(1.0f * scale, height, -1.0f * scale);
		vertex.texCoord = glm::vec2(1.0, 0.0);
		vertices.push_back(vertex);

		vertex.pos = glm::vec3(1.0f * scale, height, 1.0f * scale);
		vertex.texCoord = glm::vec2(1.0, 1.0);
		vertices.push_back(vertex);

		vertex.pos = glm::vec3(-1.0f * scale, height, 1.0f * scale);
		vertex.texCoord = glm::vec2(0.0, 1.0);
		vertices.push_back(vertex);
		
		uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
		uint32_t vertexSize = sizeof(vertices[0]);
		VkDeviceSize bufferSize = vertexSize * vertexCount;

		Buffer stagingBuffer{ device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		Helper::copyBuffer(device, stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);


	}

	void Water::createIndexBuffer() {

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(0);

		uint32_t indexCount = static_cast<uint32_t>(indices.size());
		uint32_t indexSize = sizeof(indices[0]);
		VkDeviceSize bufferSize = indexSize * indexCount;

		Buffer stagingBuffer{ device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		Helper::copyBuffer(device, stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

	}

	void Water::draw(VkCommandBuffer cmd) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	}
}