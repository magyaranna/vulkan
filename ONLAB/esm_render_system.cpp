

#include "esm_render_system.h"


namespace v {

	ESM_RenderSystem::ESM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) : device(device) {
		createShadowRenderPass();

		createShadowmapResources();
		createShadowmapDescriptorSets(descriptorSetLayout, descriptorPool);

		createPipelineLayout(setLayouts);

		createPipeline();


	}

	ESM_RenderSystem::~ESM_RenderSystem() {

		vkDestroyImageView(device.getLogicalDevice(), shadowMapESM.colorView, nullptr);
		vkDestroyImage(device.getLogicalDevice(), shadowMapESM.colorImage, nullptr);
		vkFreeMemory(device.getLogicalDevice(), shadowMapESM.colorMem, nullptr);
		vkDestroyFramebuffer(device.getLogicalDevice(), shadowMapESM.frameBuffer, nullptr);
		vkDestroyRenderPass(device.getLogicalDevice(), renderPass, nullptr);
		vkDestroySampler(device.getLogicalDevice(), shadowMapESM.sampler, nullptr);

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void ESM_RenderSystem::renderGameObjects(VkCommandBuffer& cmd, int currentFrame, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int,
		std::unique_ptr<GameObject>>&gameobjects, std::unique_ptr<Terrain> const& terrain, VkDescriptorSet shadowmap) {


		VkRenderPassBeginInfo shadowRenderPassInfo = {};
		shadowRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		shadowRenderPassInfo.renderPass = renderPass;
		shadowRenderPassInfo.framebuffer = shadowMapESM.frameBuffer;


		shadowRenderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ SHADOWMAP_DIM, SHADOWMAP_DIM };
		shadowRenderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 1> shadowClearValues = {};
		shadowClearValues[0].depthStencil = { 1.0f, 0 };

		shadowRenderPassInfo.clearValueCount = static_cast<uint32_t>(shadowClearValues.size());
		shadowRenderPassInfo.pClearValues = shadowClearValues.data();

		vkCmdBeginRenderPass(cmd, &shadowRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport v{};
		v.x = 0.0f;
		v.y = 0.0f;
		v.width = (float)extent.width;
		v.height = (float)extent.height;
		v.minDepth = 0.0f;
		v.maxDepth = 1.0f;

		VkRect2D s{};
		s.offset = { 0, 0 };
		s.extent = extent;

		vkCmdSetViewport(cmd, 0, 1, &v);
		vkCmdSetScissor(cmd, 0, 1, &s);
		{

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &shadowmap, 0, nullptr);

			vkCmdDraw(cmd, 3, 1, 0, 0);

		}

		vkCmdEndRenderPass(cmd);

	}



	void ESM_RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}
	void ESM_RenderSystem::createPipeline() {

		const std::string vert = "shaders/esmVert.spv";
		const std::string frag = "shaders/esmFrag.spv";

		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		configinfo.pipelineLayout = pipelineLayout;
		configinfo.renderPass = renderPass;

		configinfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configinfo.depthStencil.depthTestEnable = VK_FALSE;
		configinfo.depthStencil.depthWriteEnable = VK_FALSE;
		configinfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		configinfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
		configinfo.depthStencil.stencilTestEnable = VK_FALSE;

		configinfo.colorBlendAttachment.blendEnable = VK_FALSE;

		/*vertexinput*/
		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;

		configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		configinfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}



	void ESM_RenderSystem::createShadowmapResources() {

		/*color*/
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = SHADOWMAP_DIM;
		imageInfo.extent.height = SHADOWMAP_DIM;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = VK_FORMAT_R32G32_SFLOAT;

		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; 
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &shadowMapESM.colorImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), shadowMapESM.colorImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &shadowMapESM.colorMem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), shadowMapESM.colorImage, shadowMapESM.colorMem, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = shadowMapESM.colorImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R32G32_SFLOAT;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &shadowMapESM.colorView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowMapESM.colorView;
		framebufferInfo.width = SHADOWMAP_DIM;
		framebufferInfo.height = SHADOWMAP_DIM;
		framebufferInfo.layers = 1;


		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &shadowMapESM.frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}


		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(1);
		samplerInfo.mipLodBias = 0.0f; // Optional


		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &shadowMapESM.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void ESM_RenderSystem::createShadowRenderPass() {

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32_SFLOAT;   //format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassVSM{};
		subpassVSM.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassVSM.colorAttachmentCount = 1;
		subpassVSM.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependencyVSM{};
		dependencyVSM.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencyVSM.dstSubpass = 0;
		dependencyVSM.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencyVSM.srcAccessMask = 0;
		dependencyVSM.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencyVSM.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;



		VkRenderPassCreateInfo renderPassInfoVSM{};
		renderPassInfoVSM.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfoVSM.attachmentCount = 1;
		renderPassInfoVSM.pAttachments = &colorAttachment;
		renderPassInfoVSM.subpassCount = 1;
		renderPassInfoVSM.pSubpasses = &subpassVSM;
		renderPassInfoVSM.dependencyCount = 1;
		renderPassInfoVSM.pDependencies = &dependencyVSM;

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderPassInfoVSM, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void ESM_RenderSystem::createShadowmapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		shadowMapESM.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, shadowMapESM.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {


			VkDescriptorImageInfo shadowInfo{};     //shadowmap
			shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowInfo.imageView = shadowMapESM.colorView;
			shadowInfo.sampler = shadowMapESM.sampler;


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapESM.descriptorSets[i];
			descriptorWrites[0].dstBinding = 10;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &shadowInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}
}