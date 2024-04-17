

#include "buffer.h"


#include <cassert>
#include <cstring>

namespace v {

    
    Buffer::Buffer(Device& device, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
        : device{ device }, bufferSize{ bufferSize }, usageFlags{usageFlags}, memoryPropertyFlags{memoryPropertyFlags} {
        
        Helper::createBuffer(device, bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    Buffer::~Buffer() {
        unmap();
        vkDestroyBuffer(device.getLogicalDevice(), buffer, nullptr);
        vkFreeMemory(device.getLogicalDevice(), memory, nullptr);
    }


    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {

        if(size != VK_WHOLE_SIZE){
            return vkMapMemory(device.getLogicalDevice(), memory, offset, size, 0, &mapped);
        }
        return vkMapMemory(device.getLogicalDevice(), memory, offset, bufferSize, 0, &mapped);    //size!!!!!!!!
    }

    void Buffer::unmap() {
        if (mapped) {
            vkUnmapMemory(device.getLogicalDevice(), memory);
            mapped = nullptr;
        }
    }
    
    void Buffer::writeToBuffer(void* data, VkDeviceSize size) {
        if( size != VK_WHOLE_SIZE){
            memcpy(mapped, data, size);
        }
        else {
            memcpy(mapped, data, bufferSize);
        }
         
    }

    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
            buffer,
            offset,
            size,
        };
    }




   /* void Buffer::createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.getLogicalDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device.getLogicalDevice(), buffer, bufferMemory, 0);

    }


    void Buffer::copyBuffer(Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = Helper::beginSingleTimeCommands(device);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        Helper::endSingleTimeCommands(device, commandBuffer);
    }
    */
} 