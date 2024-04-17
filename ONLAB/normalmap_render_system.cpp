
#include "normalmap_render_system.h"

namespace v {

	Normalmap_RenderSystem::Normalmap_RenderSystem(Device& device, PipelineManager& pipelineManager, std::vector<VkDescriptorSetLayout> setLayouts, VkRenderPass renderPass)
		: device(device), pipelineManager(pipelineManager) {
		createPipelineLayout(setLayouts);
		createPipeline(renderPass);
		createCommandBuffers();
	}
	Normalmap_RenderSystem::~Normalmap_RenderSystem() {
		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void Normalmap_RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	void Normalmap_RenderSystem::createPipeline(VkRenderPass renderPass) {
		pipelineManager.addPipelineCreation([this, renderPass]() {
			const std::string vert = "shaders/quadVert.spv";
			const std::string frag = "shaders/normalmapFrag.spv";

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


		});
	}


	void Normalmap_RenderSystem::createCommandBuffers() {
		

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void Normalmap_RenderSystem::make_normalmap( Terrain& terrain, VkRenderPass renderPass) {

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");

		}

		VkRenderPassBeginInfo shadowRenderPassInfo = {};
		shadowRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		shadowRenderPassInfo.renderPass = renderPass;
		shadowRenderPassInfo.framebuffer = terrain.getNormalmap().framebuffer;


		shadowRenderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ terrain.getHeightMapHeight(), terrain.getHeightMapWidth()};
		shadowRenderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 1> shadowClearValues = {};
		shadowClearValues[0].depthStencil = { 1.0f, 0 };

		shadowRenderPassInfo.clearValueCount = static_cast<uint32_t>(shadowClearValues.size());
		shadowRenderPassInfo.pClearValues = shadowClearValues.data();


		vkCmdBeginRenderPass(commandBuffer, &shadowRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

		vkCmdSetViewport(commandBuffer, 0, 1, &v);
		vkCmdSetScissor(commandBuffer, 0, 1, &s);

		{

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &terrain.getHeightMapDescriptorSet(0), 0, nullptr);

			vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		}
		
		vkCmdEndRenderPass(commandBuffer);


		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit command buffer!");
		}

		if (vkQueueWaitIdle(device.getGraphicsQueue()) != VK_SUCCESS) {
			throw std::runtime_error("failed to wait for queue!");
		}
	}

}