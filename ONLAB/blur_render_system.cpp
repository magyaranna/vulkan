

#include "blur_render_system.h"

namespace v {

	BlurSystem::BlurSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout VSMLayout, VkDescriptorSetLayout layout, VkDescriptorPool pool) : device(device) {

		createRenderPass();
		createFrameBufferResources(horizontal_fb);
		createFrameBufferResources(vertical_fb);

		createSampler();

		createDescriptorSets_BluredH(VSMLayout, pool);
		createDescriptorSets(layout, pool);

		createPipelineLayout(setLayouts);

		createPipeline();
	}
	BlurSystem::~BlurSystem() {

		vkDestroyImageView(device.getLogicalDevice(), horizontal_fb.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), horizontal_fb.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), horizontal_fb.mem, nullptr);
		vkDestroyFramebuffer(device.getLogicalDevice(), horizontal_fb.frameBuffer, nullptr);

		vkDestroyImageView(device.getLogicalDevice(), vertical_fb.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), vertical_fb.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), vertical_fb.mem, nullptr);
		vkDestroyFramebuffer(device.getLogicalDevice(), vertical_fb.frameBuffer, nullptr);

		vkDestroyRenderPass(device.getLogicalDevice(), renderPass, nullptr);
		vkDestroySampler(device.getLogicalDevice(), sampler, nullptr);

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void BlurSystem::render(VkCommandBuffer& cmd, VkDescriptorSet vsmImage, int index) {

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = horizontal_fb.frameBuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ SHADOWMAP_DIM, SHADOWMAP_DIM };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 1> ClearValues = {};
		ClearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
		renderPassInfo.pClearValues = ClearValues.data();

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

		//horizontal
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			pushConstants[0] = 0;
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBlurHorz->getGraphicsPipeline());
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &vsmImage, 0, NULL);

			vkCmdDraw(cmd, 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(cmd);



		renderPassInfo.framebuffer = vertical_fb.frameBuffer;
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			pushConstants[0] = 1;
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());


			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBlurHorz->getGraphicsPipeline());
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetsBlurHorz[index], 0, NULL);

			vkCmdDraw(cmd, 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(cmd);

	}


	void BlurSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {   //sampler2d


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = setLayouts.size();
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(pushConstants);

		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}



	void BlurSystem::createPipeline() {
		const std::string vert = "shaders/gaussBlurVert.spv";
		const std::string frag = "shaders/gaussBlurFrag.spv";

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


		pipelineBlurHorz = std::make_unique<Pipeline>(device, vert, frag, configinfo);
		//pipelineBlurVert = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}

	void BlurSystem::createRenderPass() {

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

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		/*VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		*/

		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfoVSM{};
		renderPassInfoVSM.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfoVSM.attachmentCount = 1;
		renderPassInfoVSM.pAttachments = &colorAttachment;
		renderPassInfoVSM.subpassCount = 1;
		renderPassInfoVSM.pSubpasses = &subpass;
		renderPassInfoVSM.dependencyCount = 2;
		renderPassInfoVSM.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderPassInfoVSM, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

	}
	void BlurSystem::createFrameBufferResources(FrameBufferResources& fb) {

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
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &fb.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), fb.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &fb.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), fb.image, fb.mem, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = fb.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R32G32_SFLOAT;//VK_FORMAT_D32_SFLOAT_S8_UINT;              
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &fb.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}


		/*framebuffer*/

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &fb.view;
		framebufferInfo.width = SHADOWMAP_DIM;
		framebufferInfo.height = SHADOWMAP_DIM;
		framebufferInfo.layers = 1;


		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &fb.frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}



	}

	void BlurSystem::createSampler() {

		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;   /*/*/
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

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}



	void BlurSystem::createDescriptorSets_BluredH(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = descriptorPool;
		allocinfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocinfo.pSetLayouts = layouts.data();

		descriptorSetsBlurHorz.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocinfo, descriptorSetsBlurHorz.data())) {
			throw std::runtime_error("failed to allocate descr.sets!");
		}

		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = horizontal_fb.view;
			imageInfo.sampler = sampler;


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSetsBlurHorz[i];
			descriptorWrites[0].dstBinding = 9;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}

	}

	void BlurSystem::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = descriptorPool;
		allocinfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocinfo.pSetLayouts = layouts.data();

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocinfo, descriptorSets.data())) {
			throw std::runtime_error("failed to allocate descr.sets!");
		}

		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = vertical_fb.view;                                                ///!!
			imageInfo.sampler = sampler;


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 11;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}

	}


}