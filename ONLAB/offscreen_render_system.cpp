#include "offscreen_render_system.h"


namespace v {



	OffScreenRenderSystem::OffScreenRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowLayout, VkDescriptorPool pool) : device(device) {

		createShadowRenderPasses();
		/*VSM*/
		//createShadowmapResources(shadowMapVSM, VK_FORMAT_D32_SFLOAT_S8_UINT);
		//createShadowmapDescriptorSets(shadowMapVSM, shadowLayout, pool);
		
		/*sima*/
		//VkFormat depthFormat = Helper::findDepthFormat(device);
		createShadowmapResources(shadowMap);
		createShadowmapDescriptorSets(shadowMap, shadowLayout, pool);
		
		createPipelineLayout(setLayouts);
		//createPipeline(shadowMapVSM, true);
		createPipeline(shadowMap, false);
	}
	OffScreenRenderSystem::~OffScreenRenderSystem() {
		{
			vkDestroyImageView(device.getLogicalDevice(), shadowMap.view, nullptr);
			vkDestroyImage(device.getLogicalDevice(), shadowMap.image, nullptr);
			vkFreeMemory(device.getLogicalDevice(), shadowMap.mem, nullptr);
			vkDestroyFramebuffer(device.getLogicalDevice(), shadowMap.frameBuffer, nullptr);
			vkDestroyRenderPass(device.getLogicalDevice(), shadowMap.renderPass, nullptr);
			vkDestroySampler(device.getLogicalDevice(), shadowMap.sampler, nullptr);
		}
		{
			vkDestroyImageView(device.getLogicalDevice(), shadowMapVSM.view, nullptr);
			vkDestroyImage(device.getLogicalDevice(), shadowMapVSM.image, nullptr);
			vkFreeMemory(device.getLogicalDevice(), shadowMapVSM.mem, nullptr);
			vkDestroyFramebuffer(device.getLogicalDevice(), shadowMapVSM.frameBuffer, nullptr);
			vkDestroyRenderPass(device.getLogicalDevice(), shadowMapVSM.renderPass, nullptr);
			vkDestroySampler(device.getLogicalDevice(), shadowMapVSM.sampler, nullptr);
		}
		


		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


	void OffScreenRenderSystem::renderGameObjects(VkCommandBuffer& cmd, int currentFrame, bool vsm, std::unique_ptr<Light> const& light, std::unordered_map<unsigned int,
		std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain) {


		VkRenderPassBeginInfo shadowRenderPassInfo = {};
		shadowRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		
		shadowRenderPassInfo.renderPass = shadowMap.renderPass;
		shadowRenderPassInfo.framebuffer = shadowMap.frameBuffer;
		

		

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
			
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMap.pipeline->getGraphicsPipeline());


			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &light->getLightDescriptorSet(currentFrame), 0, nullptr);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &terrain->getDescriptorSet(currentFrame), 0, nullptr);
			terrain->draw(cmd);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &gameobjects.at(0)->getDescriptorSet(currentFrame), 0, nullptr);
			gameobjects.at(0)->model->draw(cmd, pipelineLayout, currentFrame, true);

		}
		
		vkCmdEndRenderPass(cmd);
		
	}


	void OffScreenRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;  
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(pushConstants);

		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pushConstantRangeCount = 1;


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}
	void OffScreenRenderSystem::createPipeline(SimpleShadowmap& sm, bool vsm) {

		const std::string vert = "shaders/shadowVert.spv";
		const std::string frag = "shaders/shadowFrag.spv";
		
		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		configinfo.pipelineLayout = pipelineLayout;
		configinfo.renderPass = sm.renderPass;

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

		sm.pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}



	void OffScreenRenderSystem::createShadowmapDescriptorSets(SimpleShadowmap& sm, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		sm.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, sm.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {


			VkDescriptorImageInfo shadowInfo{};     //shadowmap
			shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowInfo.imageView = sm.view;
			shadowInfo.sampler = sm.sampler;


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = sm.descriptorSets[i];
			descriptorWrites[0].dstBinding = 4;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &shadowInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}

	void OffScreenRenderSystem::createShadowmapResources(SimpleShadowmap& sm) {

		/*image + mem*/
		VkFormat depthFormat = Helper::findDepthFormat(device);

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
		imageInfo.format = depthFormat;                               /////////////////
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &sm.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), sm.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &sm.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), sm.image, sm.mem, 0);



		/*view*/
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = sm.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;   /**/
		viewInfo.format = depthFormat;               /**/
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &sm.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		/*framebuffer*/
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = sm.renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &sm.view;
		framebufferInfo.width = SHADOWMAP_DIM;
		framebufferInfo.height = SHADOWMAP_DIM;
		framebufferInfo.layers = 1;


		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &sm.frameBuffer) != VK_SUCCESS) {
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

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &sm.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void OffScreenRenderSystem::createShadowRenderPasses() {

		VkFormat depthFormat = Helper::findDepthFormat(device);
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; 

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 0;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;


		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &depthAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderPassInfo, nullptr, &shadowMap.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

	}
}
