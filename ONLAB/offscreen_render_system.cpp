#include "offscreen_render_system.h"


namespace v {



	OffScreenRenderSystem::OffScreenRenderSystem(Device& device, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass) : device(device) {
		ts = std::make_unique<TS_query>(device);
		createPipelineLayout(setLayouts);
		createPipeline(renderPass);
	}
	OffScreenRenderSystem::~OffScreenRenderSystem() {
		
		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


	void OffScreenRenderSystem::renderGameObjects(OffScreenRenderInfo renderinfo, DepthShadowMap& shadowMap) {


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
			if (!renderinfo.gui.frontface)
				vkCmdBindPipeline(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
			else
				vkCmdBindPipeline(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_peterpanning->getGraphicsPipeline());


			vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &renderinfo.light->getLightDescriptorSet(renderinfo.currentFrame), 0, nullptr);

			vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &renderinfo.terrain->getDefaultTextureDescriptorSets(renderinfo.currentFrame), 0, nullptr);
			vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderinfo.terrain->getDescriptorSet(renderinfo.currentFrame), 0, nullptr);
			renderinfo.terrain->draw(renderinfo.cmd);

			for (int i = 0; i < renderinfo.gameobjects.size(); i++) {
				vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderinfo.gameobjects.at(i)->getDescriptorSet(renderinfo.currentFrame), 0, nullptr);
				renderinfo.gameobjects.at(i)->model->draw(renderinfo.cmd, pipelineLayout, renderinfo.currentFrame, true, 0);
			}

		}
		//writetimestamp
		ts->writeTimeStamp(renderinfo.cmd, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

		vkCmdEndRenderPass(renderinfo.cmd);

	}


	void OffScreenRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {   //texture, modelmx, light

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}
	void OffScreenRenderSystem::createPipeline(VkRenderPass renderPass) {

		const std::string vert = "shaders/depthVert.spv";
		const std::string frag = "shaders/depthFrag.spv";

		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		configinfo.pipelineLayout = pipelineLayout;
    	configinfo.renderPass = renderPass;

		
		configinfo.rasterizer.cullMode = VK_CULL_MODE_NONE;  /*/*/
		configinfo.colorBlendAttachment.blendEnable = VK_FALSE;
		//configinfo.rasterizer.depthClampEnable = deviceFeatures.depthClamp;

		/*vertexinput*/
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(glm::vec3) + sizeof(glm::vec2);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrPos{};
		attrPos.binding = 0;
		attrPos.location = 0;
		attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrPos.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription attrUV{};
		attrUV.binding = 0;
		attrUV.location = 1;
		attrUV.format = VK_FORMAT_R32G32_SFLOAT;
		attrUV.offset = offsetof(Vertex, texCoord);


		std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.push_back(attrPos);
		attributeDescriptions.push_back(attrUV);

		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.bindingDescriptions = Vertex::getBindingDescription();

		configinfo.attributeDescriptions = attributeDescriptions;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
		configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();
		/**/

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

		configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		pipeline_peterpanning = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}

}
