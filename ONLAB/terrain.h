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


	class Terrain {
	private:

		Device& device;

		glm::vec3 offset = glm::vec3(0.0f, -10.0f, 0.0f);
		glm::vec3 scale;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unique_ptr<Buffer> vertexBuffer;
		std::unique_ptr<Buffer> indexBuffer;
		void createVertexBuffer();
		void createIndexBuffer();

		struct TerrainUniformBufferObject {
			glm::mat4 modelmx;
		};
		std::vector<std::unique_ptr<Buffer>> modelMxUniform;
		void createUniformBuffers();
		std::vector<VkDescriptorSet> descriptorSets;
		void createDescriptorSets(DescriptorSetLayout& setLayout, DescriptorPool& pool);


		TextureResources heightmap;
		FramebufferResources normalmap;
		void createHeightMapResources();
		void createNormalMapResources(VkRenderPass& renderPass);
	
		
		void generate();
		void generateFromHeightmap();
		glm::vec3 CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
		

		TextureResources grassTexture;
		TextureResources snowTexture;
		TextureResources sandTexture;
		TextureResources rockTexture;
		void createTextureResources(TextureResources& textureResources, std::string path);
		void createTextureDescriptorSet(Device& device, TextureResources& textureResources, DescriptorSetLayout& layout, DescriptorPool& pool);

	public:
		Terrain(Device& device, glm::vec3 scale, DescriptorSetLayout& setLayout, DescriptorSetLayout& textLayout, DescriptorPool& pool, VkRenderPass& renderPass);
		~Terrain();

		void updateUniformBuffer(uint32_t currentImage, bool spin);
		void draw(VkCommandBuffer cmd);


		int getHeightMapWidth(){
			return heightmap.width;
		}
		int getHeightMapHeight(){
			return heightmap.height;
		}

		FramebufferResources getNormalmap() {
			return normalmap;
		}

		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}

		VkDescriptorSet& getHeightMapDescriptorSet(int i) {
			return heightmap.descriptorSets[i];
		}

		VkDescriptorSet& getNormalMapDescriptorSet(int i) {
			return normalmap.res.descriptorSets[i];
		}

		VkDescriptorSet& getGrassTextureDescriptorSets(int i) {
			return grassTexture.descriptorSets[i];
		}
		VkDescriptorSet& getSnowTextureDescriptorSets(int i) {
			return snowTexture.descriptorSets[i];
		}
		VkDescriptorSet& getSandTextureDescriptorSets(int i) {
			return sandTexture.descriptorSets[i];
		}
		VkDescriptorSet& getRockTextureDescriptorSets(int i) {
			return rockTexture.descriptorSets[i];
		}
	};
}