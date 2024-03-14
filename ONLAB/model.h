#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>



#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "texture.h"
#include "Vertex.h"
#include "buffer.h"



namespace v {

	struct MeshPart {

		int matID;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Buffer* vertexBuffer;
		Buffer* indexBuffer;
	};

	class Model {
	private:
		Device& device;

		const std::string MODEL_PATH;

		std::vector<MeshPart> meshparts;
		std::vector<Texture*> textures;

		void createBuffers();
		void createVertexBuffer(MeshPart& meshpart);
		void createIndexBuffer(MeshPart& meshpart);

		void loadModel(DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool);

	public:
		Model(Device& device, const std::string MODEL_PATH, DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool);
		~Model();

		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, int currentframe, bool shadow, int set);
	};


}
namespace std {

	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}