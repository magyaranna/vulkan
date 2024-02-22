
#include "terrain.h"
#include <stb_image.h>

namespace v {

	Terrain::Terrain(Device& device, glm::vec3 scale, DescriptorSetLayout& setLayout, VkDescriptorSetLayout textLayout, DescriptorPool& pool) : device(device), scale(scale) {
		generate();
		createVertexBuffer();
		createIndexBuffer();

		createUniformBuffers();
		createDescriptorSets(setLayout, pool);
		defaultTextureDescriptorSets = Texture::createDefaultTextureDescriptorSet(device, defaultTexture, textLayout, pool.getDescriptorPool());

	}
	Terrain::~Terrain() {
		
		vkDestroySampler(device.getLogicalDevice(), defaultTexture.sampler, nullptr);
		vkDestroyImageView(device.getLogicalDevice(), defaultTexture.view, nullptr);
		vkDestroyImage(device.getLogicalDevice(), defaultTexture.image, nullptr);
		vkFreeMemory(device.getLogicalDevice(), defaultTexture.mem, nullptr);
	}


	void Terrain::draw(VkCommandBuffer cmd) {

		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	}

	void Terrain::generate() {

		int width, height, nChannels;
		unsigned char* data = stbi_load("textures/mo2.png",
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

		/*CALCULATE NORMALS*/
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
		return glm::normalize(glm::cross(edge1, edge2));
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


}