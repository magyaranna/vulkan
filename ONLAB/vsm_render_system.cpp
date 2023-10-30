#include "vsm_render_system.h"


namespace v {



	VSM_RenderSystem::VSM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowLayout, VkDescriptorPool pool) : device(device) {


		createShadowRenderPass();

		createShadowmapResources();
		createShadowmapDescriptorSets(shadowLayout, pool);

		createPipelineLayout(setLayouts);

		createPipeline();
	}
	VSM_RenderSystem::~VSM_RenderSystem() {

		{
			//vkDestroyImageView(device.getLogicalDevice(), shadowMapVSM.MSAAcolorView, nullptr);
			//vkDestroyImage(device.getLogicalDevice(), shadowMapVSM.MSAAcolorImage, nullptr);
			//vkFreeMemory(device.getLogicalDevice(), shadowMapVSM.MSAAcolorMem, nullptr);

			vkDestroyImageView(device.getLogicalDevice(), shadowMapVSM.colorView, nullptr);
			vkDestroyImage(device.getLogicalDevice(), shadowMapVSM.colorImage, nullptr);
			vkFreeMemory(device.getLogicalDevice(), shadowMapVSM.colorMem, nullptr);
			vkDestroyFramebuffer(device.getLogicalDevice(), shadowMapVSM.frameBuffer, nullptr);
			vkDestroyRenderPass(device.getLogicalDevice(), renderPass, nullptr);
			vkDestroySampler(device.getLogicalDevice(), shadowMapVSM.sampler, nullptr);
		}



		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


	void VSM_RenderSystem::renderGameObjects(VkCommandBuffer& cmd, int currentFrame, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int,
		std::unique_ptr<GameObject>>&gameobjects, std::unique_ptr<Terrain> const& terrain) {


		VkRenderPassBeginInfo shadowRenderPassInfo = {};
		shadowRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		shadowRenderPassInfo.renderPass = renderPass;
		shadowRenderPassInfo.framebuffer = shadowMapVSM.frameBuffer;


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

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &light->getLightDescriptorSet(currentFrame), 0, nullptr);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &terrain->getDescriptorSet(currentFrame), 0, nullptr);
			terrain->draw(cmd);

			for (int i = 0; i < gameobjects.size(); i++) {
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &gameobjects.at(i)->getDescriptorSet(currentFrame), 0, nullptr);
				gameobjects.at(i)->model->draw(cmd, pipelineLayout, currentFrame, true);
			}

		}

		vkCmdEndRenderPass(cmd);

	}


	void VSM_RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}
	void VSM_RenderSystem::createPipeline() {

		const std::string vert = "shaders/vsmVert.spv";
		const std::string frag = "shaders/vsmFrag.spv";

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

		//configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
		//configinfo.multisampling.sampleShadingEnable = VK_TRUE;
		//configinfo.multisampling.minSampleShading = .2f; 


		configinfo.colorBlendAttachment.blendEnable = VK_FALSE;

		/*vertexinput*/
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(glm::vec3);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrPos{};
		attrPos.binding = 0;
		attrPos.location = 0;
		attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrPos.offset = offsetof(Vertex, pos);


		std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.push_back(attrPos);

		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.bindingDescriptions = Vertex::getBindingDescription();

		configinfo.attributeDescriptions = attributeDescriptions;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
		configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();
		/**/

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}


	void VSM_RenderSystem::createShadowmapResources() {

		/*color*/
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = SHADOWMAP_DIM;
		imageInfo.extent.height = SHADOWMAP_DIM;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = VK_FORMAT_R32G32_SFLOAT;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &shadowMapVSM.colorImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), shadowMapVSM.colorImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &shadowMapVSM.colorMem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), shadowMapVSM.colorImage, shadowMapVSM.colorMem, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = shadowMapVSM.colorImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R32G32_SFLOAT;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &shadowMapVSM.colorView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}


		/*framebuffer*/

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowMapVSM.colorView;
		framebufferInfo.width = SHADOWMAP_DIM;
		framebufferInfo.height = SHADOWMAP_DIM;
		framebufferInfo.layers = 1;


		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &shadowMapVSM.frameBuffer) != VK_SUCCESS) {
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

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &shadowMapVSM.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	void VSM_RenderSystem::createShadowRenderPass() {

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32_SFLOAT;   //format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


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

	void VSM_RenderSystem::createShadowmapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		shadowMapVSM.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, shadowMapVSM.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {


			VkDescriptorImageInfo shadowInfo{};     //shadowmap
			shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowInfo.imageView = shadowMapVSM.colorView;
			shadowInfo.sampler = shadowMapVSM.sampler;


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapVSM.descriptorSets[i];
			descriptorWrites[0].dstBinding = 9;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &shadowInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}
}
