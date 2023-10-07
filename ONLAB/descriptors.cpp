
#include "descriptors.h"
#include <cassert>

namespace v {

    Binding& Binding::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {

        assert(bindings.count(binding) == 0 && "binding is already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;

        return *this;
    }



    /*descripotrset*/

    DescriptorSetLayout::DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : device(device) , bindings(bindings) {

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto& b : bindings) {
            setLayoutBindings.push_back(b.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(device.getLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getLogicalDevice(), descriptorSetLayout, nullptr);
    }

    
   
    /*Descriptor Pool*/

    DescriptorPool::DescriptorPool(Device& device, int count) : device(device){    //count = swapChain.MAX_FRAMES_IN_FLIGHT

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(count) * 1000;

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(count) * 1000;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(count) * 1000;

        if (vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.getLogicalDevice(), descriptorPool, nullptr);
    }



    //TODO
    bool DescriptorPool::allocateDescriptor() const {
        return true;
           
    }
   


    /*Descriptor writes*/
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) : setLayout(setLayout), pool(pool) {}

    DescriptorWriter& DescriptorWriter::writeBuffer() {
       
        return *this;
    }
    DescriptorWriter& DescriptorWriter::writeImage() {
       
        return *this;
    }

   

}