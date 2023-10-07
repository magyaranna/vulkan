

#include "gameobject.h"

namespace v {


    GameObject::GameObject(unsigned int id, Device& device, glm::vec3 s, VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool) : id(id),  device{ device } {
        scale = s;

        createUniformBuffers();

        createDescriptorSets(descriptorLayout, descriptorPool);
    
    }
	GameObject::~GameObject() {
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), modelMxUniform[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), uniformBuffersMemory[i], nullptr);
		}
	}


  
    void GameObject::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        modelMxUniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            Helper::createBuffer(device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelMxUniform[i], uniformBuffersMemory[i]);

        }
    }

   


    void GameObject::updateUniformBuffer(uint32_t currentFrame, bool spin) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};


        spin == true ?
            ubo.modelmx = glm::rotate(glm::translate(glm::mat4(1.0f), offset), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) :
            ubo.modelmx = glm::translate(glm::mat4(1.0f), offset);

        ubo.modelmx = glm::scale(ubo.modelmx, scale);

        void* data;
        vkMapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame]);
    }



   
    void GameObject::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool) {
        std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
           
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = modelMxUniform[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);   //M
         
            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 1;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;


            vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

  
}