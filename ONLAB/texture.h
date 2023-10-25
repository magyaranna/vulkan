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

namespace v {

	class Texture {

	private:

		Device& device;


		std::string texture_path;
		std::string normalmap_path;
		/**/
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		uint32_t mipLevels;

		VkImage normalMapImage;
		VkDeviceMemory normalMapImageMemory;
		VkImageView normalMapImageView;
		VkSampler normalMapSampler;

		std::vector<VkDescriptorSet> descriptorSets;


		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();

		void createNormalMapImage();
		void createNormalMapImageView();
		void createNormalMapSampler();

		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);



	public:

		Texture(Device& device, VkDescriptorSetLayout layout, VkDescriptorPool pool, std::string texture, std::string normal);
		~Texture();

		void destroy();

		void bind(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe);

		VkDescriptorSet& getDescriptorSet(int i) { return descriptorSets[i]; }
		std::vector<VkDescriptorSet> getDescriptorSets() { return descriptorSets; }

	};


}

