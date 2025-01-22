#pragma once


#include "vulkan/vulkan.h"
#include <vector>
#include "device.h"
#include "vertex.h"
#include "buffer.h"

#include "swapchain.h"
#include "descriptors.h"
#include "texture.h"

namespace v {

	

	class Water {
	private:
		Device& device;

		float scale = 10000; //400.0f;

		std::vector<Vertex> vertices;
		std::unique_ptr<Buffer> vertexBuffer;
		void createVertexBuffer();

		std::vector<uint32_t> indices;
		std::unique_ptr<Buffer> indexBuffer;
		void createIndexBuffer();


		VkRenderPass renderPass;
		OffScreenFrameBuffer reflection;
		OffScreenFrameBuffer refraction;

		TextureResources dudvTexture;
		
		void createRenderPass();
		void createOffscreenFrameBuffer(OffScreenFrameBuffer& fb, VkRenderPass renderPass,  DescriptorSetLayout& layout, DescriptorPool& pool);

		void createTextureDescriptorSet(DescriptorSetLayout& layout, DescriptorPool& pool);


	public:

		float height = 0.0f; //15.0f;
		Water(Device& device, VkRenderPass renderPass, DescriptorSetLayout& layout, DescriptorPool& pool);
		~Water();

		

		void draw(VkCommandBuffer cmd);


		OffScreenFrameBuffer getReflection() {
			return reflection;
		}
		OffScreenFrameBuffer getRefraction() {
			return refraction;
		}
		VkRenderPass getRenderPass() {
			return renderPass;
		}
		std::vector<VkDescriptorSet> getDudvTextureDescriptorSets() {
			return dudvTexture.descriptorSets;
		}
		
	};
}