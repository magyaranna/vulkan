
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
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, set, 1, &descriptorSetsTexture[currentframe], 0, nullptr);
    }

    void Texture::bindNormalMap(VkCommandBuffer cmd, VkPipelineLayout pipelinelayout, int currentframe, int set) {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, set, 1, &descriptorSetsNormalmap[currentframe], 0, nullptr);
    }


    void Texture::createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

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
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

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

        descriptorSetsTexture.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < descriptorSetsTexture.size(); i++) {
            VkDescriptorImageInfo textureImageInfo{};       //texture
            textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            textureImageInfo.imageView = texture.view;
            textureImageInfo.sampler = texture.sampler;

            DescriptorWriter(textlayout, pool)
                .createDescriptorWriter(0, &textureImageInfo)
                .build(descriptorSetsTexture[i]);
        }
      

        descriptorSetsNormalmap.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < descriptorSetsNormalmap.size(); i++) {
            VkDescriptorImageInfo textureImageInfo{};       //texture
            textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            textureImageInfo.imageView = texture.view;
            textureImageInfo.sampler = texture.sampler;

            DescriptorWriter(normallayout, pool)
                .createDescriptorWriter(0, &textureImageInfo)
                .build(descriptorSetsNormalmap[i]);
        }
       
    }

    std::vector<VkDescriptorSet> Texture::createDefaultTextureDescriptorSet(Device& device, TextureResources& textureResources, VkDescriptorSetLayout layout, VkDescriptorPool pool ) {

        std::vector<VkDescriptorSet> descSets;

        std::string texture_path = "textures/r.png";
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        int mipLevels = 1;
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Helper::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.getLogicalDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        Helper::createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureResources.image, textureResources.mem);

        Helper::transitionImageLayout(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
        Helper::copyBufferToImage(device, stagingBuffer, textureResources.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
        
        vkDestroyBuffer(device.getLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.getLogicalDevice(), stagingBufferMemory, nullptr);

        Helper::generateMipmaps(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

        textureResources.view = Helper::createImageView(device, textureResources.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);


        /*sampler*/
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

        if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &textureResources.sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }


        /*descriptorsets*/
        std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }



        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {


            VkDescriptorImageInfo textureImageInfo{};       //texture
            textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            textureImageInfo.imageView = textureResources.view;
            textureImageInfo.sampler = textureResources.sampler;

       
            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pImageInfo = &textureImageInfo;


            vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        }

        return descSets;
    }


}

