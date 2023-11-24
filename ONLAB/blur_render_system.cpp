

#include "blur_render_system.h"

namespace v {

	BlurSystem::BlurSystem(Device& device, int binding, std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout blurLayout, VkDescriptorPool pool, VkRenderPass renderPass) : device(device) {
		ts = std::make_unique<TS_query>(device);
		tempShadowMap = std::make_unique<ColorShadowMap>(device, binding, blurLayout, pool, renderPass);
		tempCascadeShadowMap = std::make_unique<ColorCascadeShadowMap>(device, binding, blurLayout, pool, renderPass);

		createPipelineLayouts(setLayouts);

		createPipelines(renderPass);
	}
	BlurSystem::~BlurSystem() {

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayoutCascade, nullptr);
	}

	
	void BlurSystem::render(VkCommandBuffer& cmd, int index, ColorShadowMap& shadowMap, VkDescriptorSet image, VkRenderPass renderPass) {

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = tempShadowMap->frameBuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ tempShadowMap->dim, tempShadowMap->dim };
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

		// Reset query pool
		ts->resetQueryPool(cmd);

		//writetimestamp
		ts->writeTimeStamp(cmd, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);


		//horizontal
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			pushConstants[0] = 0;
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &image, 0, NULL);   //amit 

			vkCmdDraw(cmd, 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(cmd);



		renderPassInfo.framebuffer = shadowMap.frameBuffer;
		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			pushConstants[0] = 1;
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());


			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &tempShadowMap->getDescriptorSet(index), 0, NULL);

			vkCmdDraw(cmd, 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(cmd);

		//writetimestamp
		ts->writeTimeStamp(cmd, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);


	}

	/*cascade*/
	void BlurSystem::render(VkCommandBuffer& cmd, int index, ColorCascadeShadowMap& shadowMap, VkDescriptorSet image, VkRenderPass renderPass) {

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;


		renderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ tempCascadeShadowMap->dim, tempCascadeShadowMap->dim };
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
		
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			renderPassInfo.framebuffer = tempCascadeShadowMap->frameBuffers[i];
			//horizontal
			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				cascadePushConstants[0] = 0;
				cascadePushConstants[1] = i;
				vkCmdPushConstants(cmd, pipelineLayoutCascade, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(cascadePushConstants), cascadePushConstants.data());

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineCascade->getGraphicsPipeline());
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutCascade, 0, 1, &image, 0, NULL);   //amit 

				vkCmdDraw(cmd, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(cmd);

			{
				VkImageMemoryBarrier imageBarrier = {};
				imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageBarrier.image = tempCascadeShadowMap->image;
				imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBarrier.subresourceRange.baseMipLevel = 0;
				imageBarrier.subresourceRange.levelCount = 1;
				imageBarrier.subresourceRange.baseArrayLayer = 0;
				imageBarrier.subresourceRange.layerCount = 4;

				vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
			}

			renderPassInfo.framebuffer = shadowMap.frameBuffers[i];
			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				cascadePushConstants[0] = 1;
				cascadePushConstants[1] = i;
				vkCmdPushConstants(cmd, pipelineLayoutCascade, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(cascadePushConstants), cascadePushConstants.data());


				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineCascade->getGraphicsPipeline());
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutCascade, 0, 1, &tempCascadeShadowMap->getDescriptorSet(index), 0, NULL);

				vkCmdDraw(cmd, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(cmd);

			
		}

	}


	void BlurSystem::createPipelineLayouts(std::vector<VkDescriptorSetLayout> setLayouts) {   


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


		VkPipelineLayoutCreateInfo cascadePipelineLayoutInfo{};
		cascadePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		cascadePipelineLayoutInfo.setLayoutCount = setLayouts.size();
		cascadePipelineLayoutInfo.pSetLayouts = setLayouts.data();

		VkPushConstantRange cascadePushConstantRange = {};
		cascadePushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		cascadePushConstantRange.offset = 0;
		cascadePushConstantRange.size = sizeof(cascadePushConstants);

		cascadePipelineLayoutInfo.pPushConstantRanges = &cascadePushConstantRange;
		cascadePipelineLayoutInfo.pushConstantRangeCount = 1;

		if (vkCreatePipelineLayout(device.getLogicalDevice(), &cascadePipelineLayoutInfo, nullptr, &pipelineLayoutCascade)) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}



	void BlurSystem::createPipelines(VkRenderPass renderPass) {
	    std::string vert = "shaders/quadVert.spv";
		std::string frag = "shaders/gaussBlurFrag.spv";

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


	    vert = "shaders/quadVert.spv";
		frag = "shaders/gaussBlurCascadeFrag.spv";

		configinfo.pipelineLayout = pipelineLayoutCascade;
	//	configinfo.renderPass = tempCascadeShadowMap->renderPass;

		pipelineCascade = std::make_unique<Pipeline>(device, vert, frag, configinfo);

	}

}