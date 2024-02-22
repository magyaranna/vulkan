#pragma once


#include "vulkan/vulkan.h"


#include <unordered_map>
#include <array>
#include "device.h"

#include "swapchain.h"



namespace v {


    struct Binding {
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        Binding& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
    };

    class DescriptorSetLayout {
    private:
        Device& device;

        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;

    public:

        DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    };
    

	class DescriptorPool {
    private:
        Device& device;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;

    public:
        DescriptorPool(Device& device, int count);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        void allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;
       // void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        VkDescriptorPool getDescriptorPool() { return descriptorPool; }

	};


    class DescriptorWriter {
    private:
        DescriptorSetLayout& setLayout;
        DescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;


    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& createDescriptorWriter(uint32_t binding, VkDescriptorBufferInfo* imageInfo);
        DescriptorWriter& createDescriptorWriter(uint32_t binding, VkDescriptorImageInfo* bufferInfo);
        void build(VkDescriptorSet& set);
        void update(VkDescriptorSet& set);
    };


}