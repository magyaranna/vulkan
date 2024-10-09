
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

    /*layout*/
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

    

    /*pool*/
    DescriptorPool::DescriptorPool(Device& device, int count) : device(device){    //count = swapChain.MAX_FRAMES_IN_FLIGHT

        std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(count) * 100;

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(count) * 100;

        poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(count) * 100;

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

    void DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const {
        
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }
   



    /**/
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) : setLayout(setLayout), pool(pool) {
    
    }

    DescriptorWriter& DescriptorWriter::createDescriptorWriter(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {

        auto& bindingDescription = setLayout.bindings[binding];

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = bindingDescription.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = bufferInfo;

        writes.push_back(descriptorWrite);
        return *this;
        
    }

    DescriptorWriter& DescriptorWriter::createDescriptorWriter(uint32_t binding, VkDescriptorImageInfo* imageInfo) {

        auto& bindingDescription = setLayout.bindings[binding];

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = bindingDescription.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = imageInfo;

        writes.push_back(descriptorWrite);
        return *this;

    }


    void DescriptorWriter::build(VkDescriptorSet& set) {
        pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        update(set);
    }

    void DescriptorWriter::update(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.getLogicalDevice(), writes.size(), writes.data(), 0, nullptr);
    }


    

}