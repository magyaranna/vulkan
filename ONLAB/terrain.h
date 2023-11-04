#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <chrono>

#include "helper.h"
#include "swapchain.h"
#include "vertex.h"
#include "texture.h"



namespace v {


	class Terrain {
	private:

		Device& device;

		glm::vec3 offset = glm::vec3(0.0f);
		glm::vec3 scale;


		struct UniformBufferObject {
			glm::mat4 modelmx;
		};
		std::vector<VkBuffer> modelMxUniform;
		std::vector<VkDeviceMemory> uniformBuffersMemory;

		std::vector<VkDescriptorSet> descriptorSets;

		
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		void createUniformBuffers();
		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);
		

		void generate();

		glm::vec3 CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
		void createVertexBuffer();
		void createIndexBuffer();

		TextureResources defaultTexture;
		std::vector<VkDescriptorSet> defaultTextureDescriptorSets;

	public:

		Terrain(Device& device, glm::vec3 scale, VkDescriptorSetLayout layout, VkDescriptorSetLayout textLayout ,VkDescriptorPool pool);
		~Terrain();


		void updateUniformBuffer(uint32_t currentImage, bool spin);
		void draw(VkCommandBuffer cmd);



		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}

		VkDescriptorSet& getDefaultTextureDescriptorSets(int i) {
			return defaultTextureDescriptorSets[i];
		}
	};
}