#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "pipeline.h"
#include <iostream>

#include <fstream>

namespace v {

    Pipeline::Pipeline(Device& device, const std::string& vert, const std::string& frag, ConfigInfo& configInfo, const std::string& tesc, const std::string& tese)
        : device{ device } {

        createGraphicsPipeline(vert, frag, configInfo, tesc, tese);
    }




    Pipeline::~Pipeline() {

        vkDestroyPipeline(device.getLogicalDevice(), pipeline, nullptr);
    }



    void Pipeline::bind(VkCommandBuffer commandBuffer) {   //TODO
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }


    void Pipeline::createGraphicsPipeline(const std::string& vert, const std::string& frag, ConfigInfo& configInfo, const std::string& tesc, const std::string& tese) {

        assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "no pipelinelayout ");
        assert(configInfo.renderPass != VK_NULL_HANDLE && "no renderapss");

        auto vertShaderCode = readFile(vert);
        auto fragShaderCode = readFile(frag);

        std::cout << vertShaderCode.size() << std::endl;
        std::cout << fragShaderCode.size() << std::endl;

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector< VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.push_back(vertShaderStageInfo);
        shaderStages.push_back(fragShaderStageInfo);

       
        VkShaderModule tescShaderModule = VK_NULL_HANDLE;
        VkShaderModule teseShaderModule = VK_NULL_HANDLE;

        if (tesc != "" && tese != "") {
            auto tescShaderCode = readFile(tesc);
            auto teseShaderCode = readFile(tese);

            tescShaderModule = createShaderModule(tescShaderCode);
            teseShaderModule = createShaderModule(teseShaderCode);

            VkPipelineShaderStageCreateInfo tescShaderStageInfo{};
            tescShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            tescShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            tescShaderStageInfo.module = tescShaderModule;
            tescShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo teseShaderStageInfo{};
            teseShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            teseShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            teseShaderStageInfo.module = teseShaderModule;
            teseShaderStageInfo.pName = "main";
            shaderStages.push_back(tescShaderStageInfo);
            shaderStages.push_back(teseShaderStageInfo);

         //   vkDestroyShaderModule(device.getLogicalDevice(), tescShaderModule, nullptr);
         //   vkDestroyShaderModule(device.getLogicalDevice(), teseShaderModule, nullptr);
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &configInfo.vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssembly;
        pipelineInfo.pTessellationState = &configInfo.tessellation;
        pipelineInfo.pViewportState = &configInfo.viewportState;
        pipelineInfo.pRasterizationState = &configInfo.rasterizer;
        pipelineInfo.pMultisampleState = &configInfo.multisampling;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencil;
        pipelineInfo.pColorBlendState = &configInfo.colorBlending;
        pipelineInfo.pDynamicState = &configInfo.dynamicState;
        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        if (tesc != "" && tese != "") {
            vkDestroyShaderModule(device.getLogicalDevice(), tescShaderModule, nullptr);
            vkDestroyShaderModule(device.getLogicalDevice(), teseShaderModule, nullptr);
        }

        vkDestroyShaderModule(device.getLogicalDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(device.getLogicalDevice(), vertShaderModule, nullptr);
    }


    void Pipeline::defaultPipelineConfigInfo(ConfigInfo& configInfo) {

        /*shader stages*/




         /*vertexinput info*/
        configInfo.bindingDescriptions = Vertex::getBindingDescription();
        configInfo.attributeDescriptions = Vertex::getAttributeDescriptions();

        configInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        configInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
        configInfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.attributeDescriptions.size());
        configInfo.vertexInputInfo.pVertexBindingDescriptions = &configInfo.bindingDescriptions;
        configInfo.vertexInputInfo.pVertexAttributeDescriptions = configInfo.attributeDescriptions.data();


        configInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

       // configInfo.tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
       // configInfo.tessellation.patchControlPoints = 1;
        

        configInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportState.viewportCount = 1;
        configInfo.viewportState.scissorCount = 1;

        configInfo.dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        configInfo.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicState.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStates.size());
        configInfo.dynamicState.pDynamicStates = configInfo.dynamicStates.data();


        configInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizer.depthClampEnable = VK_FALSE;
        configInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizer.lineWidth = 1.0f;
        configInfo.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;                      /**/
        configInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;           /**/
        configInfo.rasterizer.depthBiasEnable = VK_FALSE;


        configInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampling.sampleShadingEnable = VK_FALSE;
        configInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


        configInfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencil.depthTestEnable = VK_TRUE;
        configInfo.depthStencil.depthWriteEnable = VK_TRUE;
        configInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencil.stencilTestEnable = VK_FALSE;


        //configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        //configInfo.colorBlendAttachment.blendEnable = VK_FALSE;


        configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;

        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        /*opt*/

        configInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlending.logicOpEnable = VK_FALSE;

        configInfo.colorBlending.attachmentCount = 1;
        configInfo.colorBlending.pAttachments = &configInfo.colorBlendAttachment;

    }

    std::vector<char> Pipeline::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();


        return buffer;
    }

    VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }



}