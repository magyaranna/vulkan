#pragma once

#define NOMINMAX

#include "vulkan/vulkan.h"

#include <vector>
#include <stb_image.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "helper.h"
#include "swapchain.h"
#include "vertex.h"
#include "texture.h"

#include "buffer.h"
#include "descriptors.h"

namespace v {


	struct HeightMap {
		int width, height;
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkSampler sampler;
	};

	struct NormalMap {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkSampler sampler;
		VkFramebuffer framebuffer;
	};



	class Terrain {
	private:

		Device& device;

		glm::vec3 offset = glm::vec3(0.0f);
		glm::vec3 scale;

		

		void createHeightMapResources();
		std::vector<VkDescriptorSet> descriptorSetsHeightMap;
		void createHeightMapDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& descriptorPool);

		void createNormalMapResources(VkRenderPass& renderPass);
		std::vector<VkDescriptorSet> descriptorSetsNormalMap;
		void createNormalMapDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& descriptorPool);

		struct TerrainUniformBufferObject {
			glm::mat4 modelmx;
		};
		std::vector<std::unique_ptr<Buffer>> modelMxUniform;
		void createUniformBuffers();
	
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unique_ptr<Buffer> vertexBuffer;
		std::unique_ptr<Buffer> indexBuffer;
		void createVertexBuffer();
		void createIndexBuffer();

		std::vector<VkDescriptorSet> descriptorSets;
		void createDescriptorSets(DescriptorSetLayout& setLayout, DescriptorPool& pool);
		

		void generate();
		void generateFromHeightmap();
		glm::vec3 CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
		

		TextureResources defaultTexture;
		std::vector<VkDescriptorSet> defaultTextureDescriptorSets;

	public:
		Terrain(Device& device, glm::vec3 scale, DescriptorSetLayout& setLayout, DescriptorSetLayout& textLayout, DescriptorPool& pool, VkRenderPass& renderPass);
		~Terrain();

		HeightMap heightmap;

		NormalMap normalmap;

		void updateUniformBuffer(uint32_t currentImage, bool spin);
		void draw(VkCommandBuffer cmd);


		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}

		VkDescriptorSet& getHeightMapDescriptorSet(int i) {
			return descriptorSetsHeightMap[i];
		}

		VkDescriptorSet& getNormalMapDescriptorSet(int i) {
			return descriptorSetsNormalMap[i];
		}

		VkDescriptorSet& getDefaultTextureDescriptorSets(int i) {
			return defaultTextureDescriptorSets[i];
		}
	};
}