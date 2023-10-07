#pragma once



#include "vulkan/vulkan.h"
#include <string>
#include <vector>
#include <array>

#include "helper.h"
#include "device.h"
#include "swapchain.h"

#include "texture.h"

namespace v {

	class Material {

	private:

		Device& device;




		std::vector<Texture*> textures;

		std::vector<VkDescriptorSet> descriptorSets;



		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);


	public:




		Material(std::vector<std::string> paths);
		~Material();

		void destroy();

		void bind(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe);

		VkDescriptorSet& getDescriptorSet(int i) { return descriptorSets[i]; }
		std::vector<VkDescriptorSet> getDescriptorSets() { return descriptorSets; }

	};


}

