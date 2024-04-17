#include "offscreen_render_System.h"


namespace v {



	OffScreenRenderSystem::OffScreenRenderSystem(Device& device, TerrainRenderSystem& terrainRenderSystem,
		std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> terrainSetLayouts, VkRenderPass renderPass) : device(device), terrainRenderSystem(terrainRenderSystem) {
		createPipelineLayout(setLayouts, terrainSetLayouts);
		createPipeline(renderPass);
	}
	OffScreenRenderSystem::~OffScreenRenderSystem() {

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
		vkDestroyPipelineLayout(device.getLogicalDevice(), terrainPipelineLayout, nullptr);
	}


	void OffScreenRenderSystem::renderGameObjects(VkCommandBuffer& cmd, int currentFrame, VkRenderPass& renderPass, FramebufferResources& depthBuffer,
		Camera& camera, Terrain& terrain, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, Gui& gui, glm::vec2 viewport) {


		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = depthBuffer.framebuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent{ depthBuffer.res.width, depthBuffer.res.height };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 1> shadowClearValues = {};
		shadowClearValues[0].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(shadowClearValues.size());
		renderPassInfo.pClearValues = shadowClearValues.data();


		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
		    /*terrain*/

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline->getGraphicsPipeline());

			std::array<TerrainRenderSystem::pushConstantTesc, 1> constants1 = { {viewport, gui.tessFactor} };
			vkCmdPushConstants(cmd, terrainPipelineLayout, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 0, sizeof(TerrainRenderSystem::pushConstantTesc), constants1.data());

			std::array<TerrainRenderSystem::pushConstantTese, 1> constants2 = { gui.dFactor, glm::vec4(0.0f ,0.0f ,0.0f ,0.0f )};
			vkCmdPushConstants(cmd, terrainPipelineLayout, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 12, sizeof(TerrainRenderSystem::pushConstantTese), constants2.data());


			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 0, 1, &terrain.getGrassTextureDescriptorSets(currentFrame), 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 1, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 2, 1, &terrain.getDescriptorSet(currentFrame), 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 3, 1, &terrain.getHeightMapDescriptorSet(currentFrame), 0, nullptr);

			terrain.draw(cmd);


			/*gameobjects*/
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &terrain.getGrassTextureDescriptorSets(currentFrame), 0, nullptr);

			for (int i = 0; i < gameobjects.size(); i++) {
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &gameobjects.at(i)->getDescriptorSet(currentFrame), 0, nullptr);
				gameobjects.at(i)->model->draw(cmd, pipelineLayout, currentFrame, true, 0);
			}
		}
		
		vkCmdEndRenderPass(cmd);

	}


	void OffScreenRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> terrainSetLayouts) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkPipelineLayoutCreateInfo terrainPipelineLayoutInfo{};
		terrainPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts2 = terrainSetLayouts; 
		terrainPipelineLayoutInfo.setLayoutCount = layouts2.size();
		terrainPipelineLayoutInfo.pSetLayouts = layouts2.data();

		std::vector<VkPushConstantRange> ranges;

		
		VkPushConstantRange pushConstantRange1 = {};
		pushConstantRange1.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		pushConstantRange1.offset = 0;
		pushConstantRange1.size = sizeof(TerrainRenderSystem::pushConstantTesc);

		VkPushConstantRange pushConstantRange2 = {};
		pushConstantRange2.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		pushConstantRange2.offset = 12;
		pushConstantRange2.size = sizeof(TerrainRenderSystem::pushConstantTese);


		ranges.push_back(pushConstantRange1);
		ranges.push_back(pushConstantRange2);

		terrainPipelineLayoutInfo.pPushConstantRanges = ranges.data();
		terrainPipelineLayoutInfo.pushConstantRangeCount = ranges.size();

		if (vkCreatePipelineLayout(device.getLogicalDevice(), &terrainPipelineLayoutInfo, nullptr, &terrainPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}
	void OffScreenRenderSystem::createPipeline(VkRenderPass renderPass) {

		std::string vert = "shaders/depthVert.spv";
		std::string frag = "shaders/depthFrag.spv";

		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		configinfo.pipelineLayout = pipelineLayout;
		configinfo.renderPass = renderPass;


		configinfo.rasterizer.cullMode = VK_CULL_MODE_NONE;  
		configinfo.colorBlendAttachment.blendEnable = VK_FALSE;
		
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

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);


		vert = "shaders/terrainVert.spv";
		frag = "shaders/depthFrag.spv";
		const std::string tesc = "shaders/terrainTesc.spv";
		const std::string tese = "shaders/terrainDepthTese.spv";

		ConfigInfo configinfo2{};
		Pipeline::defaultPipelineConfigInfo(configinfo2);

		configinfo2.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		configinfo2.tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		configinfo2.tessellation.patchControlPoints = 4;

		configinfo2.renderPass = renderPass;
		configinfo2.pipelineLayout = terrainPipelineLayout;

		
		attrPos.binding = 0;
		attrPos.location = 0;
		attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrPos.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription attrNormal{};
		attrNormal.binding = 0;
		attrNormal.location = 1;
		attrNormal.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrNormal.offset = offsetof(Vertex, normal);

		attrUV.binding = 0;
		attrUV.location = 2;
		attrUV.format = VK_FORMAT_R32G32_SFLOAT;
		attrUV.offset = offsetof(Vertex, texCoord);

		std::vector< VkVertexInputAttributeDescription> attributeDescriptions2;
		attributeDescriptions2.push_back(attrPos);
		attributeDescriptions2.push_back(attrNormal);
		attributeDescriptions2.push_back(attrUV);

		configinfo2.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo2.bindingDescriptions = Vertex::getBindingDescription();
		configinfo2.attributeDescriptions = attributeDescriptions2;
		configinfo2.vertexInputInfo.vertexBindingDescriptionCount = 1;
		configinfo2.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo2.attributeDescriptions.size());
		configinfo2.vertexInputInfo.pVertexBindingDescriptions = &configinfo2.bindingDescriptions;
		configinfo2.vertexInputInfo.pVertexAttributeDescriptions = configinfo2.attributeDescriptions.data();

		terrainPipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo2, tesc, tese);

	}

}
