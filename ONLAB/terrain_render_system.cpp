
#include "terrain_render_system.h"


namespace v {

    TerrainRenderSystem::TerrainRenderSystem(Device& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts) : device(device) {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);
    }



    TerrainRenderSystem::~TerrainRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
    }

    void TerrainRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts; //camera, terrain modelmx, light , shadowmap,,cascadeShadowmap, cascadeuniform
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(pushConstants);

        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }


    void TerrainRenderSystem::createPipeline(VkRenderPass renderPass) {

        const std::string vert = "shaders/terrainVert.spv";
        const std::string frag = "shaders/terrainFrag.spv";

        ConfigInfo configinfo{};
        Pipeline::defaultPipelineConfigInfo(configinfo);

        configinfo.pipelineLayout = pipelineLayout;
        configinfo.renderPass = renderPass;

        configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
        configinfo.multisampling.sampleShadingEnable = VK_TRUE;
        configinfo.multisampling.minSampleShading = .2f;

        VkVertexInputAttributeDescription attrPos{};
        attrPos.binding = 0;
        attrPos.location = 0;
        attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
        attrPos.offset = offsetof(Vertex, pos);

        VkVertexInputAttributeDescription attrNormal{};
        attrNormal.binding = 0;
        attrNormal.location = 1;
        attrNormal.format = VK_FORMAT_R32G32B32_SFLOAT;
        attrNormal.offset = offsetof(Vertex, normal);


        std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.push_back(attrPos);
        attributeDescriptions.push_back(attrNormal);

        configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        configinfo.bindingDescriptions = Vertex::getBindingDescription();
        configinfo.attributeDescriptions = attributeDescriptions;
        configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
        configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
        configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
        configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();




        pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
    }


    void TerrainRenderSystem::renderTerrain(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());


        renderInfo.gui.displayNormalmap == true ? pushConstants[0] = 1 : pushConstants[0] = 0;
        renderInfo.gui.cascade == true ? pushConstants[1] = 1 : pushConstants[1] = 0;
        renderInfo.gui.vsm == true ? pushConstants[2] = 1 : pushConstants[2] = 0;
        renderInfo.gui.esm == true ? pushConstants[3] = 1 : pushConstants[3] = 0;
        renderInfo.gui.cascadecolor == true ? pushConstants[4] = 1 : pushConstants[4] = 0;
        renderInfo.gui.pcf == true ? pushConstants[5] = 1 : pushConstants[5] = 0;
        renderInfo.gui.bias == true ? pushConstants[6] = 1 : pushConstants[6] = 0;
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &renderInfo.camera->getDescriptorSet(currentFrame), 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &renderInfo.light->getLightDescriptorSet(currentFrame), 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &renderInfo.simpleShadowMap, 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &renderInfo.cascadeShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 5, 1, &renderInfo.cascadeLightSpaceMx, 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 6, 1, &renderInfo.vsmShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 7, 1, &renderInfo.esmShadowmap, 0, nullptr);


        //modelmx
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderInfo.terrain->getDescriptorSet(currentFrame), 0, nullptr);

        renderInfo.terrain->draw(cmd);
    }

}