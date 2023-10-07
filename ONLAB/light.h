#pragma once


#include "vulkan/vulkan.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <array>

#include "helper.h"

#include "swapchain.h"



namespace v {

	class Light {
	private:

		Device& device;

		glm::vec4 position = glm::vec4(10.0f, 50.0f, 50.0f, 1.0f); // glm::vec4(10.0f, 50.0f, 50.0f, 1.0f);
		glm::vec3 dir = glm::vec3(0.0f, -0.8f, -1.0f);
		

		std::vector<VkBuffer> lightUniformBuffers;
		std::vector<VkDeviceMemory> lightUniformBuffersMemory;

		std::vector<VkBuffer> lightVPUniformBuffers;
		std::vector<VkDeviceMemory> lightVPUniformBuffersMemory;

		std::vector<VkDescriptorSet> lightDescriptorSets;
		

		
		void createLightUniformBuffers();
		void createLightVPUniformBuffers();

		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);
		
	public:

		Light(Device& device, VkDescriptorSetLayout layout, VkDescriptorPool pool);
		~Light();

		struct UniformBufferLightVP {
			glm::mat4 view;
			glm::mat4 proj;
		};
		struct UniformBufferLight {	
			glm::vec4 lightPosition;
			glm::vec3 lightDir;

			//+ 
		};

		void updateLightUniformBuffer(uint32_t currentImage);
		void updateLightVPUniformBuffer(uint32_t currentImage);
		

		void setDirection(glm::vec3 value) { dir = value; }

		/*getters*/
		VkBuffer getLightUniformBuffer(int i) { return lightUniformBuffers[i]; }
		VkBuffer getLightVPUniformBuffer(int i) { return lightVPUniformBuffers[i]; }
		glm::vec4 getPos() { return position; }
		glm::vec3 getDir() { return dir; }

		VkDescriptorSet& getLightDescriptorSet(int i) {
			return lightDescriptorSets[i];
		}
		

		
	};


}