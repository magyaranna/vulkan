
#include "terrain.h"
#include <stb_image.h>

namespace v {

	Terrain::Terrain(Device& device, glm::vec3 scale, VkDescriptorSetLayout layout, VkDescriptorPool pool) : device(device), scale(scale) {
		generate();
		createVertexBuffer();
		createIndexBuffer();

		createUniformBuffers();
		createDescriptorSets(layout, pool);


	}
	Terrain::~Terrain() {
		vkDestroyBuffer(device.getLogicalDevice(), indexBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(device.getLogicalDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), vertexBufferMemory, nullptr);

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), modelMxUniform[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), uniformBuffersMemory[i], nullptr);
		}
	}


	void Terrain::draw(VkCommandBuffer cmd) {


		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);


	}

	void Terrain::generate() {


		int width, height, nChannels;
		unsigned char* data = stbi_load("textures/mo.png",
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

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

		Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		Helper::copyBuffer(device, stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);

	}
	void Terrain::createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

		Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		Helper::copyBuffer(device, stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);
	}


	/**/
	void Terrain::createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		modelMxUniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelMxUniform[i], uniformBuffersMemory[i]);
		}
	}




	void Terrain::updateUniformBuffer(uint32_t currentFrame, bool spin) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};


		//spin == true ?
			//ubo.modelmx = glm::rotate(glm::translate(glm::mat4(1.0f), offset), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) :
		ubo.modelmx = glm::translate(glm::mat4(1.0f), offset);

		ubo.modelmx = glm::scale(ubo.modelmx, scale);

		void* data;
		vkMapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame]);
	}

	void Terrain::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = modelMxUniform[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);   //M

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 1;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;


			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

}