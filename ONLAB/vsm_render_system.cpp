#include "vsm_render_system.h"


namespace v {



	VSM_RenderSystem::VSM_RenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass) : device(device) {

		ts = std::make_unique<TS_query>(device);
		createPipelineLayout(setLayouts);

		createPipeline(renderPass);
	}
	VSM_RenderSystem::~VSM_RenderSystem() {

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


	void VSM_RenderSystem::renderGameObjects(OffScreenRenderInfo renderinfo, ColorShadowMap& shadowMap, VkDescriptorSet descriptorSetShadowmap) {


		VkRenderPassBeginInfo shadowRenderPassInfo = {};
		shadowRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		shadowRenderPassInfo.renderPass = renderinfo.renderPass;
		shadowRenderPassInfo.framebuffer = shadowMap.frameBuffer;


		shadowRenderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ shadowMap.dim, shadowMap.dim };
		shadowRenderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 1> shadowClearValues = {};
		shadowClearValues[0].depthStencil = { 1.0f, 0 };

		shadowRenderPassInfo.clearValueCount = static_cast<uint32_t>(shadowClearValues.size());
		shadowRenderPassInfo.pClearValues = shadowClearValues.data();

		// Reset query pool
		ts->resetQueryPool(renderinfo.cmd);

		vkCmdBeginRenderPass(renderinfo.cmd, &shadowRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

		vkCmdSetViewport(renderinfo.cmd, 0, 1, &v);
		vkCmdSetScissor(renderinfo.cmd, 0, 1, &s);

		//writetimestamp
		ts->writeTimeStamp(renderinfo.cmd, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		{
			
			vkCmdBindPipeline(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

			vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetShadowmap, 0, nullptr);

			vkCmdDraw(renderinfo.cmd, 3, 1, 0, 0);

		}

		//writetimestamp
		ts->writeTimeStamp(renderinfo.cmd, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

		vkCmdEndRenderPass(renderinfo.cmd);

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

	void VSM_RenderSystem::createPipeline(VkRenderPass renderPass) {

		const std::string vert = "shaders/quadVert.spv";
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
		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;

		configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		configinfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}


}
