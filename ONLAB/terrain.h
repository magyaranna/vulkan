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




namespace v {

	/*struct TerrainVertex {
		glm::vec3 pos;
		glm::vec3 normal;
		
		static VkVertexInputBindingDescription getBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const TerrainVertex& other) const;
	};*/



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

	public:

		Terrain(Device& device, glm::vec3 scale, VkDescriptorSetLayout layout, VkDescriptorPool pool);
		~Terrain();


		void updateUniformBuffer(uint32_t currentImage, bool spin);
		void draw(VkCommandBuffer cmd);



		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}
	};
}