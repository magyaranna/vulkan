#pragma once


#include "device.h"
#include "helper.h"

namespace v {

    class Buffer {
    private:

        Device& device;
        void* mapped = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        
        VkDeviceSize bufferSize;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;

    public:
        VkDeviceMemory memory = VK_NULL_HANDLE;


        Buffer( Device& device, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);


        //static void createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        //static void copyBuffer(Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


        VkBuffer getBuffer() const { return buffer; }
        void* getMappedMemory() const { return mapped; }


    };

} 