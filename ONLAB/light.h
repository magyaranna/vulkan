#pragma once


#include "vulkan/vulkan.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <array>

#include "helper.h"

#include "swapchain.h"
#include "buffer.h"
#include "descriptors.h"



namespace v {

	class Light {
	private:

		Device& device;

		glm::vec3 position =  glm::vec3(0.0f, 50.0f, 0.0f); 
		glm::vec3 dir = glm::vec3(-0.5f, -1.0f, -0.5f);

		std::vector<std::unique_ptr<Buffer>> lightUniform;
		void createUniformBuffers();

		std::vector<VkDescriptorSet> descriptorSets;
		void createDescriptorSets(DescriptorSetLayout& descriptorSetLayout, DescriptorPool& descriptorPool);

	public:

		Light(Device& device, DescriptorSetLayout& layout, DescriptorPool& pool);
		~Light();

		struct LightUniformBuffer {
			glm::mat4 view;
			glm::mat4 proj;
			glm::vec3 lightPosition;
			glm::vec3 lightDir;
		};
		void updateUniformBuffer(uint32_t currentImage);
		

		void setDirection(glm::vec3 value) { dir = value; }

		/*getters*/
		VkBuffer getLightUniformBuffer(int i) { return lightUniform[i]->getBuffer(); }
		glm::vec3 getPos() { return position; }
		glm::vec3 getDir() { return dir; }

		VkDescriptorSet& getLightDescriptorSet(int i) {
			return descriptorSets[i];
		}



	};


}