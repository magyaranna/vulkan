

#include "skybox.h"

namespace v {


    SkyBox::SkyBox(Device& device, DescriptorSetLayout& layout, DescriptorPool& descriptorPool) : device(device) {
       filenames = {"textures/skybox/right.png",
                        "textures/skybox/left.png", 
                        "textures/skybox/top.png", 
                        "textures/skybox/bottom.png", 
                        "textures/skybox/back.png", 
                        "textures/skybox/front.png" };
      /*   filenames = {"textures/alps/right.png",
                        "textures/alps/left.png", 
                        "textures/alps/top.png", 
                        "textures/alps/bottom.png", 
                        "textures/alps/back.png", 
                        "textures/alps/front.png" };*/
        
        load();
        createVertexBuffer();
        createDescriptorSets(layout, descriptorPool);
    }

    SkyBox::~SkyBox() {
        vkDestroySampler(device.getLogicalDevice(), skybox.sampler, nullptr);
        vkDestroyImageView(device.getLogicalDevice(), skybox.view, nullptr);
        vkDestroyImage(device.getLogicalDevice(), skybox.image, nullptr);
        vkFreeMemory(device.getLogicalDevice(), skybox.mem, nullptr);

    }


    void SkyBox::draw(VkCommandBuffer cmd) {

        VkBuffer buffers[] = { vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdDraw(cmd, 36, 1, 0, 0);
        
    }


    void SkyBox::createVertexBuffer() {


        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        
        int idx = 0;
        for (int i = 0; i < 36; i++) {
            Vertex vertex{};
            vertex.pos = glm::vec3(skyboxVertices[3 * i] * 100, skyboxVertices[3 * i + 1] * 100, skyboxVertices[3 * i +2]*100);
            vertices.push_back(vertex);
            
        }

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer{ device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };


        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<Buffer>(device, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Helper::copyBuffer(device, stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

    }
   

    void SkyBox::createDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& descriptorPool) {

        descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < descriptorSets.size(); i++) {

            VkDescriptorImageInfo skyboxImageInfo{};
            skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            skyboxImageInfo.imageView = skybox.view;
            skyboxImageInfo.sampler = skybox.sampler;
           
            DescriptorWriter(layout, descriptorPool)
                .createDescriptorWriter(0, &skyboxImageInfo)
                .build(descriptorSets[i]);

        }
    }


    void SkyBox::load() {

        stbi_uc* pixels[6];

        int texWidth, texHeight, texChannels;
        for (int i = 0; i < 6; ++i) {
            pixels[i] = stbi_load(filenames[i], &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }
        }
        skybox.width = texWidth;
        skybox.height = texHeight;
        size_t imageSize = texWidth * texHeight * 4;
        size_t inputSize = imageSize * 6;

        Buffer stagingBuffer{ device, inputSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        
        for (int i = 0; i < 6; i++) {
            stagingBuffer.map(imageSize, imageSize * i);
            stagingBuffer.writeToBuffer((void*)pixels[i], static_cast<size_t>(imageSize));
            stagingBuffer.unmap();
            stbi_image_free(pixels[i]);
        }

        createResources();
       
        Helper::transitionImageLayout(device, skybox.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 6);
        Helper::copyBufferToImage(device, stagingBuffer.getBuffer(), skybox.image, static_cast<uint32_t>(skybox.width), static_cast<uint32_t>(texHeight), 6);
        Helper::transitionImageLayout(device, skybox.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 6);
    }


    void SkyBox::createResources() {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.extent.width = skybox.width;
        imageInfo.extent.height = skybox.height;
        imageInfo.extent.depth = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;   
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.mipLevels = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        

        if (vkCreateImage(device.getLogicalDevice(), &imageInfo, nullptr, &skybox.image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create  image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.getLogicalDevice(), skybox.image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &skybox.mem) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device.getLogicalDevice(), skybox.image, skybox.mem, 0);


        /*view*/
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = skybox.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;
        viewInfo.subresourceRange.levelCount = 1;

        if (vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &skybox.view) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        /*sampler*/
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;              
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 200.0f;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &skybox.sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

      
    }
}
