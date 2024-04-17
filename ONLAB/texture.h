#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <vector>
#include <array>

#include "helper.h"
#include "device.h"
#include "swapchain.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "descriptors.h"

namespace v {

	class Texture {
	private:
		Device& device;

		std::string texture_path;
		std::string normalmap_path;
		/**/
		TextureResources texture;
		TextureResources normalmap;

		uint32_t mipLevels;

		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();

		void createNormalMapImage();
		void createNormalMapImageView();
		void createNormalMapSampler();

		void createDescriptorSets(DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool);

	public:

		Texture(Device& device, DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool, std::string texture, std::string normal);
		~Texture();

		void destroy();

		void bindTexture(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe, int set);
		void bindNormalMap(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe, int set);


		VkDescriptorSet& getDescriptorSetTexture(int i) { return texture.descriptorSets[i]; }
		VkDescriptorSet& getDescriptorSetNormalmap(int i) { return normalmap.descriptorSets[i]; }

		std::vector<VkDescriptorSet> getTextureDescriptorSets() {
			return texture.descriptorSets;
		};
	};


}

