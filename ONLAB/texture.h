#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <vector>
#include <array>

#include "helper.h"
#include "device.h"
#include "swapchain.h"


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

		void createDescriptorSets( VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);


		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	public:

		Texture(Device& device, VkDescriptorSetLayout layout, VkDescriptorPool pool,  std::string texture,  std::string normal);
		~Texture();

		void destroy();

		void bind(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe);

		VkDescriptorSet& getDescriptorSet(int i) { return descriptorSets[i]; }
		std::vector<VkDescriptorSet> getDescriptorSets() { return descriptorSets; }

	};


}

