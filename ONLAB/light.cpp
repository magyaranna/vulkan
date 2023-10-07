
#include "light.h"
#include <stdexcept>
#include <fstream>

namespace v {


	Light::Light(Device& device, VkDescriptorSetLayout layout, VkDescriptorPool pool) :device{ device } {

		createLightUniformBuffers();
		createLightVPUniformBuffers();
		

		createDescriptorSets(layout, pool);
		
	}
	Light::~Light() {

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), lightUniformBuffers[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), lightUniformBuffersMemory[i], nullptr);
		}
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), lightVPUniformBuffers[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), lightVPUniformBuffersMemory[i], nullptr);
		}
	}

	

	
    void Light::createLightUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferLight);

        lightUniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        lightUniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
       

        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            Helper::createBuffer(device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightUniformBuffers[i], lightUniformBuffersMemory[i]);
        }
    }
    void Light::createLightVPUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferLightVP);

		lightVPUniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        lightVPUniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            Helper::createBuffer(device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightVPUniformBuffers[i], lightVPUniformBuffersMemory[i]);
        }
    }

	void Light::updateLightUniformBuffer(uint32_t currentImage) {
			
		UniformBufferLight ubl = {};	 
		ubl.lightPosition = position;
		ubl.lightDir = dir;

		void* data;
		vkMapMemory(device.getLogicalDevice(), lightUniformBuffersMemory[currentImage], 0, sizeof(ubl), 0, &data);
		memcpy(data, &ubl, sizeof(ubl));
		vkUnmapMemory(device.getLogicalDevice(), lightUniformBuffersMemory[currentImage]);
	}

	void Light::updateLightVPUniformBuffer(uint32_t currentImage) {

		UniformBufferLightVP ubs = {};	
	
		                                     
		
		//ubs.proj = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, 0.1f, 70.0f);  //50.0f
		ubs.proj = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, 0.1f, 200.0f);  



		ubs.view = glm::lookAt(glm::vec3(position.x, position.y, position.z),
			glm::vec3(position.x, position.y, position.z) + dir  ,
			glm::vec3(0.0f, 1.0f, 0.0f));

		
		void* data;
		vkMapMemory(device.getLogicalDevice(), lightVPUniformBuffersMemory[currentImage], 0, sizeof(ubs), 0, &data);
		memcpy(data, &ubs, sizeof(ubs));
		vkUnmapMemory(device.getLogicalDevice(), lightVPUniformBuffersMemory[currentImage]);
	}

	

	void Light::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {

		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		lightDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, lightDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
		
			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = lightVPUniformBuffers[i];
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = 2 * sizeof(glm::mat4);   //light VP    

			VkDescriptorBufferInfo lightPosBufferInfo{};
			lightPosBufferInfo.buffer = lightUniformBuffers[i];
			lightPosBufferInfo.offset = 0;
			lightPosBufferInfo.range = sizeof(UniformBufferLight);   //light

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = lightDescriptorSets[i];
			descriptorWrites[0].dstBinding = 2;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &lightBufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = lightDescriptorSets[i];
			descriptorWrites[1].dstBinding = 3;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &lightPosBufferInfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}
	}
	


	
}