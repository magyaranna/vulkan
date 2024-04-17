
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_map>
#include <stdexcept>

namespace v {

    Texture::Texture(Device& device, DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool, std::string texture, std::string normal) : device{ device } {

        texture_path = texture;
        normalmap_path = normal;


        if (texture_path != "" && normalmap_path != "") {
            createTextureImage();
            createTextureImageView();
            createTextureSampler();

            createNormalMapImage();
            createNormalMapImageView();
            createNormalMapSampler();

            createDescriptorSets(textlayout, normallayout, pool);
        }


    }
    Texture::~Texture() {

    }


    void Texture::destroy() {
        vkDestroySampler(device.getLogicalDevice(), texture.sampler, nullptr);
        vkDestroyImageView(device.getLogicalDevice(), texture.view, nullptr);
        vkDestroyImage(device.getLogicalDevice(), texture.image, nullptr);
        vkFreeMemory(device.getLogicalDevice(), texture.mem, nullptr);

        vkDestroySampler(device.getLogicalDevice(), normalmap.sampler, nullptr);
        vkDestroyImageView(device.getLogicalDevice(), normalmap.view, nullptr);
        vkDestroyImage(device.getLogicalDevice(), normalmap.image, nullptr);
        vkFreeMemory(device.getLogicalDevice(), normalmap.mem, nullptr);
    }


    void Texture::bindTexture(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe, int set) {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, set, 1, &texture.descriptorSets[currentframe], 0, nullptr);
    }

    void Texture::bindNormalMap(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe, int set) {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, set, 1, &normalmap.descriptorSets[currentframe], 0, nullptr);
    }


    void Texture::createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        texture.height = texHeight;
        texture.width = texWidth;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Helper::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        Helper::createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.mem);

        Helper::transitionImageLayout(device, texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
        Helper::copyBufferToImage(device, stagingBuffer, texture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
        // transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);

        Helper::generateMipmaps(device, texture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
    }


    void Texture::createTextureImageView() {
        texture.view = Helper::createImageView(device, texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    }
    void Texture::createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f; // Optional
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f; // Optional

        if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }


    void Texture::createNormalMapImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(normalmap_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        normalmap.height = texHeight;
        normalmap.width = texWidth;
        

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Helper::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        Helper::createImage(device, texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalmap.image, normalmap.mem);

        Helper::transitionImageLayout(device, normalmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
        Helper::copyBufferToImage(device, stagingBuffer, normalmap.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
        Helper::transitionImageLayout(device, normalmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

        vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void Texture::createNormalMapImageView() {
        normalmap.view = Helper::createImageView(device, normalmap.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    void Texture::createNormalMapSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &normalmap.sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }



    void Texture::createDescriptorSets(DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool) {

        texture.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < texture.descriptorSets.size(); i++) {
            VkDescriptorImageInfo textureImageInfo{};       //texture
            textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            textureImageInfo.imageView = texture.view;
            textureImageInfo.sampler = texture.sampler;

            DescriptorWriter(textlayout, pool)
                .createDescriptorWriter(0, &textureImageInfo)
                .build(texture.descriptorSets[i]);
        }
      

        normalmap.descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < normalmap.descriptorSets.size(); i++) {
            VkDescriptorImageInfo textureImageInfo{};       //texture
            textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            textureImageInfo.imageView = texture.view;
            textureImageInfo.sampler = texture.sampler;

            DescriptorWriter(normallayout, pool)
                .createDescriptorWriter(0, &textureImageInfo)
                .build(normalmap.descriptorSets[i]);
        }
       
    }

    

}

