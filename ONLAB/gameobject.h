#pragma once
#include <vector>
#include <array>
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <chrono>

#include "helper.h"
#include "device.h"
#include "swapchain.h"
#include "model.h"



namespace v {


	class GameObject {
	private:
		

		Device& device;
		
		
		glm::vec3 offset = glm::vec3(0.0f);
		glm::vec3 scale;

		unsigned int id;
	
		std::vector<VkBuffer> modelMxUniform;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		
		std::vector<VkDescriptorSet> descriptorSets;
		

		void createUniformBuffers();
		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);
	
	public:
		
		GameObject(unsigned int id,  Device& device,glm::vec3 scale,VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);
		~GameObject();

		std::shared_ptr<Model> model{};

		struct UniformBufferObject {
			glm::mat4 modelmx;
		};
		
		void updateUniformBuffer(uint32_t currentImage, bool spin);
		
		/*getters*/
		VkBuffer getUniformBuffers(size_t i) { return modelMxUniform[i]; }
		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}
		unsigned int getId() { return id; }
	};

}