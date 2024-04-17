

#include "gameobject.h"

namespace v {


    GameObject::GameObject(unsigned int id, Device& device, glm::vec3 scale, glm::vec3 offset, DescriptorSetLayout& layout, DescriptorPool& pool) 
        : id(id), device{ device }, scale{ scale }, offset{ offset } {
        createUniformBuffers();

        createDescriptorSets(layout, pool);

    }
    GameObject::~GameObject() {

    }



    void GameObject::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(GameObjectUniformBUffer);

        modelMxUniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < modelMxUniform.size(); i++) {
            modelMxUniform[i] = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            modelMxUniform[i]->map();
        }
    }




    void GameObject::updateUniformBuffer(uint32_t currentFrame, bool spin) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        GameObjectUniformBUffer ubo{};


        spin == true ?
            ubo.modelmx = glm::rotate(glm::translate(glm::mat4(1.0f), offset), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) :
            ubo.modelmx = glm::translate(glm::mat4(1.0f), offset);

       
            /*macsk*/
           // ubo.modelmx = glm::rotate(glm::translate(glm::mat4(1.0f), offset), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
           // ubo.modelmx = glm::rotate(ubo.modelmx, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.modelmx = glm::scale(ubo.modelmx, scale);

        modelMxUniform[currentFrame]->writeToBuffer((void*)&ubo);
    }




    void GameObject::createDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& pool) {
        descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < descriptorSets.size(); i++) {

            auto bufferinfo = modelMxUniform[i]->descriptorInfo(sizeof(GameObjectUniformBUffer));

            DescriptorWriter(layout, pool)
                .createDescriptorWriter(1, &bufferinfo)
                .build(descriptorSets[i]);
        }
    }


}