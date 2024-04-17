#pragma once


#include "vulkan/vulkan.h"
#include <stb_image.h>


#include "helper.h"
#include "vertex.h"
#include "swapchain.h"
#include "buffer.h"
#include "descriptors.h"


namespace v {



	class SkyBox {
	private:
		Device& device;

		TextureResources skybox;
		std::vector<const char*> filenames;

		std::vector<Vertex> vertices;
		std::unique_ptr<Buffer> vertexBuffer;
		void createVertexBuffer();
		

		void load();
		void createResources();
		void createDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& descriptorPool);

	public:

		SkyBox(Device& device, DescriptorSetLayout& layout, DescriptorPool& descriptorPool);
		~SkyBox();

		void draw(VkCommandBuffer cmd);

		VkDescriptorSet& getDescriptorSet(int i) {
			return skybox.descriptorSets[i];
		}
	};

}

