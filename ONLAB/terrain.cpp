
#include "terrain.h"

namespace v {

	Terrain::Terrain(Device& device, glm::vec3 scale, DescriptorSetLayout& setLayout, DescriptorSetLayout& textLayout, DescriptorPool& pool) : device(device), scale(scale) {
		generate();
		//generateFromHeightmap();
		createVertexBuffer();
		createIndexBuffer();

		createUniformBuffers();
		createDescriptorSets(setLayout, pool);
		defaultTextureDescriptorSets = Texture::createDefaultTextureDescriptorSet(device, defaultTexture, textLayout.getDescriptorSetLayout(), pool.getDescriptorPool());


		createHeightMapDescriptorSets(textLayout, pool);

	}
	Terrain::~Terrain() {
		
		vkDestroySampler(device.getLogicalDevice(), defaultTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), defaultTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), defaultTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), defaultTexture.mem, nullptr);



		vkDestroySampler(device.getLogicalDevice(), heightmap.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), heightmap.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), heightmap.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), heightmap.mem, nullptr);
	}


	void Terrain::draw(VkCommandBuffer cmd) {

		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	}

	void Terrain::generate() {

		unsigned char* pixels;
		int texWidth, texHeight, texChannels;
		
		pixels = stbi_load("textures/render.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
				throw std::runtime_error("failed to load texture image!");
		}

		
		heightmap.width = texWidth;
		heightmap.height = texHeight;
		size_t imageSize = texWidth * texHeight * 4;

		Buffer stagingBuffer{ device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		
		stagingBuffer.map(imageSize, 0);
		stagingBuffer.writeToBuffer((void*)pixels, static_cast<size_t>(imageSize));
		stagingBuffer.unmap();
		
		createHeightMapResources();

		Helper::transitionImageLayout(device, heightmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		Helper::copyBufferToImage(device, stagingBuffer.getBuffer(), heightmap.image, static_cast<uint32_t>(heightmap.width), static_cast<uint32_t>(texHeight), 1);
		Helper::transitionImageLayout(device, heightmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

	
		const uint32_t patchSize{ 64 };
		const float uvScale{ 1.0f };
		uint32_t dim;
		uint32_t scale;
		
		const float wx = 2.0f;
		const float wy = 2.0f;

		dim = heightmap.width;

		uint16_t* heightdata;
		heightdata = reinterpret_cast<uint16_t*>(pixels);

		scale = dim / patchSize;
		
		//Generate vertices
		const uint32_t vertexCount = patchSize * patchSize;
		Vertex tempVertices[vertexCount];

		for (auto x = 0; x < patchSize; x++) {
			for (auto y = 0; y < patchSize; y++) {
				uint32_t index = (x + y * patchSize);
				Vertex vertex{};
				vertex.pos = glm::vec3(
					x * wx + wx / 2.0f -(float)patchSize * wx / 2.0f,
					0.0f,
					y * wy + wy / 2.0f - (float)patchSize * wy / 2.0f);
				vertex.texCoord = glm::vec2((float)x / (patchSize - 1), (float)y / (patchSize - 1)) * uvScale;
				tempVertices[index] = vertex;
			}
		}
		vertices.insert(vertices.end(), tempVertices, tempVertices + vertexCount);

		
		//Generate normals
		for (auto x = 0; x < patchSize; x++) {
			for (auto y = 0; y < patchSize; y++) {
	
				float heights[3][3];
				for (auto sx = -1; sx <= 1; sx++) {
					for (auto sy = -1; sy <= 1; sy++) {
						glm::ivec2 rpos = glm::ivec2(x + sx, y + sy) * glm::ivec2(scale);
						rpos.x = std::max(0, std::min(rpos.x, (int)dim - 1));
						rpos.y = std::max(0, std::min(rpos.y, (int)dim - 1));
						rpos /= glm::ivec2(scale);
						
						//heights[sx + 1][sy + 1] = tomb[rpos.x + rpos.y * dim] * scale;
						heights[sx + 1][sy + 1] =(float) *(heightdata + (rpos.x + rpos.y * dim) * scale ) / 65535.0f;
					}
				}
				glm::vec3 normal;

				normal.x = heights[0][0] - heights[2][0] + 2.0f * heights[0][1] - 2.0f * heights[2][1] + heights[0][2] - heights[2][2];

				normal.z = heights[0][0] + 2.0f * heights[1][0] + heights[2][0] - heights[0][2] - 2.0f * heights[1][2] - heights[2][2];
				
				normal.y = 0.25f * sqrt(1.0f - normal.x * normal.x - normal.z * normal.z);

				vertices[x + y * patchSize].normal = glm::normalize(normal * glm::vec3(2.0f, 1.0f, 2.0f));
			}
		}

		// Generate indices
		const uint32_t w = (patchSize - 1);
		const uint32_t indexCount = w * w * 4;
		uint32_t* tempIndicies = new uint32_t[indexCount];
		for (auto x = 0; x < w; x++)
		{
			for (auto y = 0; y < w; y++)
			{
				uint32_t index = (x + y * w) * 4;
				tempIndicies[index] = (x + y * patchSize);
				tempIndicies[index + 1] = tempIndicies[index] + patchSize;
				tempIndicies[index + 2] = tempIndicies[index + 1] + 1;
				tempIndicies[index + 3] = tempIndicies[index] + 1;
			}
		}
		indices.insert(indices.end(), tempIndicies, tempIndicies + indexCount);

	}

	void Terrain::createHeightMapResources() {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = heightmap.width;
		imageInfo.extent.height = heightmap.height;
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;


		if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &heightmap.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create  image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getLogicalDevice(), heightmap.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &heightmap.mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.getLogicalDevice(), heightmap.image, heightmap.mem, 0);


		/*view*/
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = heightmap.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &heightmap.view) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
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

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &heightmap.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}



	void Terrain::createHeightMapDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& descriptorPool) {

		descriptorSetsHeightMap.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < descriptorSetsHeightMap.size(); i++) {

			VkDescriptorImageInfo skyboxImageInfo{};
			skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			skyboxImageInfo.imageView = heightmap.view;
			skyboxImageInfo.sampler = heightmap.sampler;

			DescriptorWriter(layout, descriptorPool)
				.createDescriptorWriter(0, &skyboxImageInfo)
				.build(descriptorSetsHeightMap[i]);

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


