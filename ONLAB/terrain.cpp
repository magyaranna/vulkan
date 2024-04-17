
#include "terrain.h"

namespace v {

	Terrain::Terrain(Device& device, glm::vec3 scale, DescriptorSetLayout& setLayout, DescriptorSetLayout& textLayout, DescriptorPool& pool, VkRenderPass& renderPass) : device(device), scale(scale) {
		generate();
		createVertexBuffer();
		createIndexBuffer();

		createUniformBuffers();
		createDescriptorSets(setLayout, pool);

		createTextureDescriptorSet(device, heightmap, textLayout, pool);
		createNormalMapResources(renderPass);
		createTextureDescriptorSet(device, normalmap.res, textLayout, pool);

		createTextureResources(grassTexture, "textures/terrain/terrain.jpg");
		createTextureDescriptorSet(device, grassTexture, textLayout, pool);
		createTextureResources(snowTexture, "textures/terrain/snow.bmp");
		createTextureDescriptorSet(device,  snowTexture, textLayout, pool);
		createTextureResources(sandTexture, "textures/terrain/sand.bmp");
		createTextureDescriptorSet(device, sandTexture, textLayout, pool);
		createTextureResources(rockTexture, "textures/terrain/rock.bmp");
		createTextureDescriptorSet(device,  rockTexture, textLayout, pool);


		


	}
	Terrain::~Terrain() {

		vkDestroySampler(device.getLogicalDevice(), grassTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), grassTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), grassTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), grassTexture.mem, nullptr);

		vkDestroySampler(device.getLogicalDevice(), snowTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), snowTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), snowTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), snowTexture.mem, nullptr);

		vkDestroySampler(device.getLogicalDevice(), sandTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), sandTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), sandTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), sandTexture.mem, nullptr);

		vkDestroySampler(device.getLogicalDevice(), rockTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), rockTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), rockTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), rockTexture.mem, nullptr);



		vkDestroySampler(device.getLogicalDevice(), heightmap.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), heightmap.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), heightmap.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), heightmap.mem, nullptr);

		vkDestroySampler(device.getLogicalDevice(), normalmap.res.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), normalmap.res.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), normalmap.res.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), normalmap.res.mem, nullptr);
		vkDestroyFramebuffer(device.getLogicalDevice(), normalmap.framebuffer, nullptr);
	}


	void Terrain::draw(VkCommandBuffer cmd) {

		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	}

	void Terrain::generate() {

		unsigned char* pixels;
		int texWidth, texHeight, texChannels;

		pixels = stbi_load("textures/heightmap2.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		heightmap.width = texWidth;
		heightmap.height = texHeight;

;
		size_t imageSize = texWidth * texHeight * 4;

		Buffer stagingBuffer{ device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };


		stagingBuffer.map(imageSize, 0);
		stagingBuffer.writeToBuffer((void*)pixels, static_cast<size_t>(imageSize));
		stagingBuffer.unmap();

		createHeightMapResources();

		Helper::transitionImageLayout(device, heightmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		Helper::copyBufferToImage(device, stagingBuffer.getBuffer(), heightmap.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		Helper::transitionImageLayout(device, heightmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

	
		const uint32_t rez{ 10 };
		const float uvScale{ 1.0f };
		uint32_t dim;
		uint32_t scale;

		const float wx = 100.0f;
		const float wy = 100.0f;

		dim = heightmap.width;

		scale = dim / rez;

		//Generate vertices
		const uint32_t vertexCount = rez * rez;
		Vertex tempVertices[vertexCount];

		for (auto x = 0; x < rez; x++) {
			for (auto y = 0; y < rez; y++) {
				uint32_t index = (x + y * rez);
				Vertex vertex{};
				vertex.pos = glm::vec3(
					x * wx + wx / 2.0f - (float)rez * wx / 2.0f,
					0.0f,
					y * wy + wy / 2.0f - (float)rez * wy / 2.0f);
				vertex.texCoord = glm::vec2((float)x / (rez - 1), (float)y / (rez - 1)) * uvScale;
				tempVertices[index] = vertex;
			}
		}
		vertices.insert(vertices.end(), tempVertices, tempVertices + vertexCount);



		// Generate indices
		const uint32_t w = (rez - 1);
		const uint32_t indexCount = w * w * 4;
		uint32_t* tempIndicies = new uint32_t[indexCount];
		for (auto x = 0; x < w; x++)
		{
			for (auto y = 0; y < w; y++)
			{
				uint32_t index = (x + y * w) * 4;
				tempIndicies[index] = (x + y * rez);
				tempIndicies[index + 1] = tempIndicies[index] + rez;
				tempIndicies[index + 2] = tempIndicies[index + 1] + 1;
				tempIndicies[index + 3] = tempIndicies[index] + 1;
			}
		}
		indices.insert(indices.end(), tempIndicies, tempIndicies + indexCount);
	}

	void Terrain::createHeightMapResources() {

		Helper::createImage(device, heightmap.width, heightmap.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, heightmap.image, heightmap.mem);

		heightmap.view = Helper::createImageView(device, heightmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 200.0f;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &heightmap.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	void Terrain::createNormalMapResources(VkRenderPass& renderPass) {

		normalmap.res.width = heightmap.width;
		normalmap.res.height = heightmap.height;

		Helper::createImage(device, normalmap.res.width, normalmap.res.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalmap.res.image, normalmap.res.mem);

		normalmap.res.view = Helper::createImageView(device, normalmap.res.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		/*framebuffer*/
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &normalmap.res.view;
		framebufferInfo.width = heightmap.width;
		framebufferInfo.height = heightmap.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(device.getLogicalDevice(), &framebufferInfo, nullptr, &normalmap.framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}


		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 200.0f;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &normalmap.res.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	void Terrain::createVertexBuffer() {

		uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
		uint32_t vertexSize = sizeof(vertices[0]);
		VkDeviceSize bufferSize = vertexSize * vertexCount;

		Buffer stagingBuffer{ device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		Helper::copyBuffer(device, stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

	}
	void Terrain::createIndexBuffer() {

		uint32_t indexCount = static_cast<uint32_t>(indices.size());
		uint32_t indexSize = sizeof(indices[0]);
		VkDeviceSize bufferSize = indexSize * indexCount;

		Buffer stagingBuffer{ device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(device, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		Helper::copyBuffer(device, stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

	}

	/**/
	void Terrain::createUniformBuffers() {

		modelMxUniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDeviceSize bufferSize = sizeof(TerrainUniformBufferObject);

		for (int i = 0; i < modelMxUniform.size(); i++) {
			modelMxUniform[i] = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			modelMxUniform[i]->map();
		}
	}

	
	void Terrain::updateUniformBuffer(uint32_t currentFrame, bool spin) {

		TerrainUniformBufferObject ubo{};
		ubo.modelmx = glm::translate(glm::mat4(1.0f), offset);
		ubo.modelmx = glm::scale(ubo.modelmx, scale);


		//modelMxUniform[currentFrame]->map(); 
		modelMxUniform[currentFrame]->writeToBuffer(&ubo);
	}


	void Terrain::createDescriptorSets(DescriptorSetLayout& setLayout, DescriptorPool& pool) {
		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < descriptorSets.size(); i++) {

			auto bufferInfo = modelMxUniform[i]->descriptorInfo(sizeof(TerrainUniformBufferObject), 0);

			DescriptorWriter(setLayout, pool)
				.createDescriptorWriter(1, &bufferInfo)
				.build(descriptorSets[i]);
		}
	}

	void Terrain::createTextureResources(TextureResources& textureResources, std::string path) {
		std::string texture_path = path;
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		size_t imageSize = texWidth * texHeight * 4;
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		textureResources.height = texHeight;
		textureResources.width = texWidth;


		Buffer stagingBuffer{ device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		stagingBuffer.map(imageSize);
		stagingBuffer.writeToBuffer((void*)pixels);
		stagingBuffer.unmap();
		stbi_image_free(pixels);

		Helper::createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureResources.image, textureResources.mem);

		Helper::transitionImageLayout(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
		Helper::copyBufferToImage(device, stagingBuffer.getBuffer(), textureResources.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
		Helper::generateMipmaps(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

		textureResources.view = Helper::createImageView(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


		/*sampler*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &textureResources.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void Terrain::createTextureDescriptorSet(Device& device, TextureResources& textureResources,
		DescriptorSetLayout& layout, DescriptorPool& pool) {

		textureResources.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < textureResources.descriptorSets.size(); i++) {

			VkDescriptorImageInfo skyboxImageInfo{};
			skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			skyboxImageInfo.imageView = textureResources.view;
			skyboxImageInfo.sampler = textureResources.sampler;

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &skyboxImageInfo)
				.build(textureResources.descriptorSets[i]);

		}
	}



	////////////////////////////////////////////////////////

	void Terrain::generateFromHeightmap() {

		int width, height, nChannels;
		unsigned char* data = stbi_load("textures/render.png",
			&width, &height, &nChannels,
			0);

		float yScale = 64.0f / 256.0f, yShift = 16.0f;
		int rez = 1;
		unsigned bytePerPixel = nChannels;

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
				unsigned char y = pixelOffset[0];

				Vertex vertex{};

				vertex.pos = glm::vec3(-height / 2.0f + height * i / (float)height, (int)y * yScale - yShift, -width / 2.0f + width * j / (float)width);
				//vertex.normal = glm::vec3(0.0f);
				vertices.push_back(vertex);
			}
		}
		stbi_image_free(data);

		for (int y = 0; y < height - 1; ++y) {
			for (int x = 0; x < width - 1; ++x) {
				uint32_t topLeft = y * width + x;
				uint32_t topRight = topLeft + 1;
				uint32_t bottomLeft = (y + 1) * width + x;
				uint32_t bottomRight = bottomLeft + 1;

				indices.push_back(topLeft);
				indices.push_back(topRight);
				indices.push_back(bottomLeft);

				indices.push_back(bottomLeft);
				indices.push_back(topRight);
				indices.push_back(bottomRight);
			}
		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			uint32_t index1 = indices[i];
			uint32_t index2 = indices[i + 1];
			uint32_t index3 = indices[i + 2];

			glm::vec3 v1 = vertices[index1].pos;
			glm::vec3 v2 = vertices[index2].pos;
			glm::vec3 v3 = vertices[index3].pos;

			glm::vec3 normal = CalculateNormal(v1, v2, v3);

			vertices[index1].normal += normal;
			vertices[index2].normal += normal;
			vertices[index3].normal += normal;
		}

		for (size_t i = 0; i < vertices.size(); ++i) {
			vertices[i].normal = glm::normalize(vertices[i].normal);
		}

	}

	glm::vec3 Terrain::CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
		glm::vec3 edge1 = v2 - v1;
		glm::vec3 edge2 = v3 - v1;
		glm::vec3 a =glm::cross(edge1, edge2);
		glm::vec3 b = glm::normalize(a);
		return b;
	}
}


