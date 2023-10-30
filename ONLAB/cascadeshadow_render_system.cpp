
#include "cascadeshadow_render_system.h"


namespace v {

	CascadeShadowRenderSystem::CascadeShadowRenderSystem(Device& device,
		std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowmapLayout, VkDescriptorPool descriptorPool, glm::vec4 lightPos) : device(device) {

		createRenderPass();
		createShadowmapResources();


		createPipelineLayout(setLayouts);
		createPipeline();

		createUniformBuffers();
		createDescriptorSets(setLayouts[0], descriptorPool);

		createShadowMapDescriptorSets(shadowmapLayout, descriptorPool, device.getLogicalDevice());


	}
	CascadeShadowRenderSystem::~CascadeShadowRenderSystem() {

		for (int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			vkDestroyImageView(device.getLogicalDevice(), cascades[i].view, nullptr);/**/
			vkDestroyFramebuffer(device.getLogicalDevice(), cascades[i].frameBuffer, nullptr);/**/
		}

		vkDestroyImage(device.getLogicalDevice(), shadowMap.image, nullptr);/**/
		vkFreeMemory(device.getLogicalDevice(), shadowMap.mem, nullptr);/**/
		vkDestroyImageView(device.getLogicalDevice(), shadowMap.view, nullptr);/**/
		vkDestroySampler(device.getLogicalDevice(), shadowMap.sampler, nullptr);


		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), uniformbuffer[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), uniformbufferMem[i], nullptr);
		}

		vkDestroyRenderPass(device.getLogicalDevice(), renderPass, nullptr);/**/

		vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);


	}
	void CascadeShadowRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

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
	}

	void CascadeShadowRenderSystem::createPipeline() {
		const std::string vert = "shaders/cascadeVert.spv";
		const std::string frag = "shaders/cascadeFrag.spv";

		ConfigInfo configinfo{};
		Pipeline::defaultPipelineConfigInfo(configinfo);

		//colorBlendState.attachmentCount = 0;
		//depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		configinfo.pipelineLayout = pipelineLayout;
		configinfo.renderPass = renderPass;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 0;  //1
		colorBlending.pAttachments = nullptr;

		configinfo.colorBlending = colorBlending;

		/*vertexinput*/
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(glm::vec3);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.offset = offsetof(Vertex, pos);

		std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.push_back(attrDesc);


		configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		configinfo.bindingDescriptions = Vertex::getBindingDescription();
		configinfo.attributeDescriptions = attributeDescriptions;
		configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
		configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
		configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
		configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();


		pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
	}


	//1db cascadenal az osszes gameobject kirajzolasa
	void CascadeShadowRenderSystem::renderScene(VkCommandBuffer commandBuffer, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain,
		uint32_t cascadeIndex, int currentFrame) {

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

		pushConstants[0] = cascadeIndex;
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), pushConstants.data());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &MXDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &terrain->getDescriptorSet(currentFrame), 0, nullptr);
		terrain->draw(commandBuffer);

		for (int i = 0; i < gameobjects.size(); i++) {
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &gameobjects.at(i)->getDescriptorSet(currentFrame), 0, nullptr);
			gameobjects.at(i)->model->draw(commandBuffer, pipelineLayout, currentFrame, true);
		}

	}


	void CascadeShadowRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain,
		int currentFrame) {
		VkClearValue clearValues[1];
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		VkExtent2D extent{ SHADOWMAP_DIM, SHADOWMAP_DIM };
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

		vkCmdSetViewport(commandBuffer, 0, 1, &v);
		vkCmdSetScissor(commandBuffer, 0, 1, &s);

		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			renderPassBeginInfo.framebuffer = cascades[i].frameBuffer;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				renderScene(commandBuffer, gameobjects, terrain, i, currentFrame);
			}
			vkCmdEndRenderPass(commandBuffer);
		}
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

	void CascadeShadowRenderSystem::updateCascades(std::unique_ptr<Camera> const& camera, glm::vec3 dir) {
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
			float d = cascadeSplitLambda * (log - uniform) + uniform;             //ci = lambda * ci_log + (1-lambda)* ci_uni
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
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);


			cascades[i].splitDepth = (nearclip + splitDist * clipRange); //*-1.0f;
			cascades[i].viewProjMx = lightOrthoMatrix * lightViewMatrix;


			lastSplitDist = cascadeSplits[i];
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


	void CascadeShadowRenderSystem::createShadowMapDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool, VkDevice device) {
		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		shadowMap.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, shadowMap.descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {


			VkDescriptorImageInfo shadowInfo{};     //shadowmap
			shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowInfo.imageView = shadowMap.view;
			shadowInfo.sampler = shadowMap.sampler;

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMap.descriptorSets[i];
			descriptorWrites[0].dstBinding = 7;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &shadowInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
			descriptorWrites[0].dstBinding = 8;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}



	void CascadeShadowRenderSystem::createRenderPass() {
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = Helper::findDepthFormat(device);
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


		std::array<VkSubpassDependency, 2> dependencies; /**/

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

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void CascadeShadowRenderSystem::createShadowmapResources() {

		/*image + mem*/
		VkFormat depthFormat = Helper::findDepthFormat(device);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = SHADOWMAP_DIM;
		imageInfo.extent.height = SHADOWMAP_DIM;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = depthFormat;                               /////////////////
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &shadowMap.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), shadowMap.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &shadowMap.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), shadowMap.image, shadowMap.mem, 0);



		/*view*/
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = shadowMap.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;   /**/
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = SHADOW_MAP_CASCADE_COUNT;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &shadowMap.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		/*per cascade view + framebuffer*/
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = shadowMap.image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &cascades[i].view) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
			// Framebuffer
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &cascades[i].view;
			framebufferInfo.width = SHADOWMAP_DIM;
			framebufferInfo.height = SHADOWMAP_DIM;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &cascades[i].frameBuffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &shadowMap.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}







