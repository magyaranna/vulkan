
#include "cascadeshadow_render_system.h"


namespace v {

	CascadeShadowRenderSystem::CascadeShadowRenderSystem(Device& device,
		std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> setLayoutsESM, VkDescriptorPool pool, VkRenderPass rp, VkRenderPass colorRp) : device(device) {
		ts = std::make_unique<TS_query>(device);
	
		createPipelineLayout(setLayouts, setLayoutsESM);
		createPipeline(rp, colorRp);

		createUniformBuffers();
		createDescriptorSets(setLayouts[2], pool);

	}
	CascadeShadowRenderSystem::~CascadeShadowRenderSystem() {
		
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), uniformbuffer[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), uniformbufferMem[i], nullptr);
		}


		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
		vkDestroyPipelineLayout(device.getLogicalDevice(), colorPipelineLayout, nullptr);


	}
	void CascadeShadowRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> setLayoutsColor) {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts = setLayouts;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(pushConstants);

		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


		VkPipelineLayoutCreateInfo pipelineLayoutInfoColor{};
		pipelineLayoutInfoColor.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layoutsColor = setLayoutsColor;
		pipelineLayoutInfoColor.setLayoutCount = layoutsColor.size();
		pipelineLayoutInfoColor.pSetLayouts = layoutsColor.data();


		VkPushConstantRange pushConstantRangeColor = {};
		pushConstantRangeColor.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRangeColor.offset = 0;
		pushConstantRangeColor.size = sizeof(pushConstants);

		pipelineLayoutInfoColor.pPushConstantRanges = &pushConstantRangeColor;
		pipelineLayoutInfoColor.pushConstantRangeCount = 1;


		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfoColor, nullptr, &colorPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


	}

	void CascadeShadowRenderSystem::createPipeline(VkRenderPass renderPass, VkRenderPass colorRenderPass) {
		std::string vert = "shaders/depthCascadeVert.spv";
		std::string frag = "shaders/depthFrag.spv";

		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		configinfo.pipelineLayout = pipelineLayout;
		configinfo.renderPass = renderPass;

		configinfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configinfo.rasterizer.cullMode = VK_CULL_MODE_NONE;  /*/*/

		/*vertexinput*/
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(glm::vec3) + sizeof(glm::vec2);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.offset = offsetof(Vertex, pos);

		VkVertexInputAttributeDescription attrUV{};
		attrUV.binding = 0;
		attrUV.location = 1;
		attrUV.format = VK_FORMAT_R32G32_SFLOAT;
		attrUV.offset = offsetof(Vertex, texCoord);

		std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.push_back(attrDesc);
		attributeDescriptions.push_back(attrUV);


		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.bindingDescriptions = Vertex::getBindingDescription();
		configinfo.attributeDescriptions = attributeDescriptions;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
		configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();

		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

		
		/******/
		vert = "shaders/quadVert.spv";
		frag = "shaders/vcsmFrag.spv";

		configinfo.pipelineLayout = colorPipelineLayout;
		configinfo.renderPass = colorRenderPass;

		configinfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configinfo.depthStencil.depthTestEnable = VK_FALSE;
		configinfo.depthStencil.depthWriteEnable = VK_FALSE;
		configinfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		configinfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
		configinfo.depthStencil.stencilTestEnable = VK_FALSE;

		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
		configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;

		configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		configinfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		pipelineVSM = std::make_unique<Pipeline>(device, vert, frag, configinfo);


		/******/
		vert = "shaders/quadVert.spv";
		frag = "shaders/ecsmFrag.spv";


		pipelineESM = std::make_unique<Pipeline>(device, vert, frag, configinfo);
		
	}

	void CascadeShadowRenderSystem::renderGameObjects(OffScreenRenderInfo renderinfo, CascadeShadowMap& shadowMap) {

		VkClearValue clearValues[1];
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderinfo.renderPass;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		VkExtent2D extent{ shadowMap.dim, shadowMap.dim };
		renderPassBeginInfo.renderArea.extent = extent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

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

		// Reset query pool
		//ts->resetQueryPool(renderinfo.cmd);
		//writetimestamp
		//ts->writeTimeStamp(renderinfo.cmd, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			renderPassBeginInfo.framebuffer = shadowMap.frameBuffers[i];
			vkCmdBeginRenderPass(renderinfo.cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
				renderScene(renderinfo, i);
			}
			vkCmdEndRenderPass(renderinfo.cmd);
		}
		//writetimestamp
		//ts->writeTimeStamp(renderinfo.cmd, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

	//1db cascadenal az osszes gameobject kirajzolasa
	void CascadeShadowRenderSystem::renderScene(OffScreenRenderInfo renderinfo, int cascadeIndex) {

		pushConstants[0] = cascadeIndex;
		vkCmdPushConstants(renderinfo.cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), pushConstants.data());

		vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &MXDescriptorSets[renderinfo.currentFrame], 0, nullptr);

		vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &renderinfo.terrain->getDefaultTextureDescriptorSets(renderinfo.currentFrame), 0, nullptr);
		vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderinfo.terrain->getDescriptorSet(renderinfo.currentFrame), 0, nullptr);
		renderinfo.terrain->draw(renderinfo.cmd);

		for (int i = 0; i < renderinfo.gameobjects.size(); i++) {
			vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderinfo.gameobjects.at(i)->getDescriptorSet(renderinfo.currentFrame), 0, nullptr);
			renderinfo.gameobjects.at(i)->model->draw(renderinfo.cmd, pipelineLayout, renderinfo.currentFrame, true, 0);
		}

	}


	/*color*/
	void CascadeShadowRenderSystem::renderGameObjects(OffScreenRenderInfo renderinfo, ColorCascadeShadowMap& shadowMap, CascadeShadowMap& depthshadowmap) {

		VkClearValue clearValues[1];
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderinfo.renderPass;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		VkExtent2D extent{ shadowMap.dim, shadowMap.dim };
		renderPassBeginInfo.renderArea.extent = extent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

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

		// Reset query pool
		ts->resetQueryPool(renderinfo.cmd);
		//writetimestamp
		ts->writeTimeStamp(renderinfo.cmd, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);


		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			renderPassBeginInfo.framebuffer = shadowMap.frameBuffers[i];
			vkCmdBeginRenderPass(renderinfo.cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderinfo.gui.vsm ? pipelineVSM->getGraphicsPipeline() : pipelineESM->getGraphicsPipeline());
				renderScene(renderinfo, i, depthshadowmap);
			}
			vkCmdEndRenderPass(renderinfo.cmd);
		}
		//writetimestamp
		ts->writeTimeStamp(renderinfo.cmd, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

	}
	void CascadeShadowRenderSystem::renderScene(OffScreenRenderInfo renderinfo, int cascadeIndex, CascadeShadowMap& shadowMap) {

		pushConstants[0] = cascadeIndex;
		vkCmdPushConstants(renderinfo.cmd, colorPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());

		vkCmdBindDescriptorSets(renderinfo.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipelineLayout, 0, 1, &shadowMap.getDescriptorSet(renderinfo.currentFrame), 0, nullptr);

		vkCmdDraw(renderinfo.cmd, 3, 1, 0, 0);
	}



	void CascadeShadowRenderSystem::createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformLightSpace);

		uniformbuffer.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		uniformbufferMem.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);


		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformbuffer[i], uniformbufferMem[i]);
		}
	}

	void CascadeShadowRenderSystem::updateUniformBuffers(uint32_t currentImage) {
		UniformLightSpace ubo = {};

		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			ubo.cascadeViewProjMat[i] = cascades[i].viewProjMx;
			ubo.cascadeSplits[i] = cascades[i].splitDepth;      /**/
		}

		void* data;
		vkMapMemory(device.getLogicalDevice(), uniformbufferMem[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device.getLogicalDevice(), uniformbufferMem[currentImage]);
	}



	//https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus

	//https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/

	void CascadeShadowRenderSystem::updateCascades(std::unique_ptr<Camera> const& camera, glm::vec3 dir, float splitLambda) {
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

		float clipRange = camera->getFarClip() - camera->getNearClip();
		float nearclip = camera->getNearClip();

		float minZ = nearclip;
		float maxZ = nearclip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;


		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);                                //ci_log = near * ratio^p
			float uniform = minZ + range * p;                                     //ci_uniform = near + range*p
			float d = splitLambda * (log - uniform) + uniform;             //ci = lambda * ci_log + (1-lambda)* ci_uni
			cascadeSplits[i] = (d - nearclip) / clipRange;    //[0,1]
		}


		float lastSplitDist = 0.0;

		//1db cascade-ra
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			//corners-> world space   iit megkapjuk az egesz nezetablak szeleit
			glm::mat4 invCam = glm::inverse(camera->matrices.proj * camera->matrices.view);
			for (uint32_t i = 0; i < 8; i++) {
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			//
			for (uint32_t i = 0; i < 4; i++) {
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}


			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;


			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = glm::normalize(dir);
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			float r =  2.0f * glm::vec3(radius).z;
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, r);

			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			shadowOrigin = shadowMatrix * shadowOrigin;
			shadowOrigin = shadowOrigin * 3000.0f / 2.0f;

			glm::vec4 roundedOrigin = glm::round(shadowOrigin);
			glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
			roundOffset = roundOffset * 2.0f / 3000.0f;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			glm::mat4 shadowProj = lightOrthoMatrix;
			shadowProj[3] += roundOffset;
			lightOrthoMatrix = shadowProj;
			
			cascades[i].splitDepth = (nearclip + splitDist * clipRange) * -1.0f; //*-1.0f;
			cascades[i].viewProjMx = lightOrthoMatrix * lightViewMatrix;
			

			lastSplitDist = cascadeSplits[i];
		}
	}


	void CascadeShadowRenderSystem::createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool) {
		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		MXDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, MXDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformbuffer[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformLightSpace);

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = MXDescriptorSets[i];
			descriptorWrites[0].dstBinding = 5;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}


	/*	void CascadeShadowRenderSystem::updateCascades(std::unique_ptr<Camera> const& camera, glm::vec3 dir) {

			//std::vector<float> shadowCascadeLevels{ camera->getFarClip() / 50.0f, camera->getFarClip() / 25.0f, camera->getFarClip() / 2.0f };
			std::vector<float> shadowCascadeLevels{ 20.0f, 50.0f, 80.0f };
			std::vector<glm::mat4> viewproj = getLightSpaceMatrices(camera, dir, shadowCascadeLevels);

			for (int i = 0; i < 4; i++) {
				if(i == 3) cascades[i].splitDepth = camera->getFarClip();
				else cascades[i].splitDepth = shadowCascadeLevels[i];
				cascades[i].viewProjMx = viewproj[i];
			}

		}*/

	std::vector<glm::mat4> CascadeShadowRenderSystem::getLightSpaceMatrices(std::unique_ptr<Camera> const& camera, glm::vec3 dir, std::vector<float> shadowCascadeLevels)
	{
		std::vector<glm::mat4> ret;
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
		{
			if (i == 0)
			{
				ret.push_back(getLightSpaceMatrix(camera, dir, camera->getNearClip(), shadowCascadeLevels[i]));
			}
			else if (i < shadowCascadeLevels.size())
			{
				ret.push_back(getLightSpaceMatrix(camera, dir, shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
			}
			else
			{
				ret.push_back(getLightSpaceMatrix(camera, dir, shadowCascadeLevels[i - 1], camera->getFarClip()));
			}
		}
		return ret;
	}

	glm::mat4 CascadeShadowRenderSystem::getLightSpaceMatrix(std::unique_ptr<Camera> const& camera, glm::vec3 dir, const float nearPlane, const float farPlane)
	{
		const auto proj = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, nearPlane, farPlane);
		const auto projview = proj * camera->matrices.view;
		const auto inv = glm::inverse(projview);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : frustumCorners)
		{
			center += glm::vec3(v);
		}
		center /= frustumCorners.size();

		dir = glm::normalize(dir);

		const auto lightView = glm::lookAt(center - dir, center, glm::vec3(0.0f, 1.0f, 0.0f));    //* (farPlane - nearPlane)

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : frustumCorners)
		{
			const auto trf = lightView * v;    //to lightspace
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}


		constexpr float zMult = 10.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		//const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, 0.0f, maxZ - minZ);
		return lightProjection * lightView;
	}
}







