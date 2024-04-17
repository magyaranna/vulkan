#include "render_system.h"

namespace v {

    RenderSystem::RenderSystem(Device& device, PipelineManager& pipelineManager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts) : device(device), pipelineManager(pipelineManager){
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);

    }
    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
    }


    void RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts;  //globalDescriptorSetLayout, texture,normalmap, gameobjDescriptorSetLayout, lightDescriptorSetLayout, shadowmapDescriptorSetLayout ,cascadeShadowmap, cascadeuniform
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();


        std::vector<VkPushConstantRange> ranges;
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(pushConstants);

        VkPushConstantRange pushConstantRange2 = {};
        pushConstantRange2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange2.offset = 32;
        pushConstantRange2.size = sizeof(glm::vec4);

        ranges.push_back(pushConstantRange);
        ranges.push_back(pushConstantRange2);

        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
        pipelineLayoutInfo.pushConstantRangeCount = ranges.size();


        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }



    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        pipelineManager.addPipelineCreation([this, renderPass] {
            const std::string vert = "shaders/sceneVert.spv";
            const std::string frag = "shaders/sceneFrag.spv";

            ConfigInfo configinfo{};
            Pipeline::defaultPipelineConfigInfo(configinfo);

            configinfo.pipelineLayout = pipelineLayout;
            configinfo.renderPass = renderPass;
            configinfo.rasterizer.cullMode = VK_CULL_MODE_NONE;

            configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
            configinfo.multisampling.sampleShadingEnable = VK_TRUE;
            configinfo.multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother


            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.bindingDescriptions = Vertex::getBindingDescription();
            configinfo.attributeDescriptions = Vertex::getAttributeDescriptions();
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
            configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();


            pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);
           });
        

    }



    void RenderSystem::renderGameObjects(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderinfo, Camera& camera, glm::vec4 clipPlane) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

        //TODO
        renderinfo.gui.displayNormalmap == true ? pushConstants[0] = 1 : pushConstants[0] = 0;
        renderinfo.gui.cascade == true ? pushConstants[1] = 1 : pushConstants[1] = 0;
        renderinfo.gui.vsm == true ? pushConstants[2] = 1 : pushConstants[2] = 0;
        renderinfo.gui.esm == true ? pushConstants[3] = 1 : pushConstants[3] = 0;
        renderinfo.gui.cascadecolor == true ? pushConstants[4] = 1 : pushConstants[4] = 0;
        renderinfo.gui.pcf == true ? pushConstants[5] = 1 : pushConstants[5] = 0;
        renderinfo.gui.bias == true ? pushConstants[6] = 1 : pushConstants[6] = 0;
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants), pushConstants.data());

        std::array<glm::vec4, 1> constants1 = { clipPlane };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 32, sizeof(glm::vec4), constants1.data());


        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &renderinfo.light->getLightDescriptorSet(currentFrame), 0, nullptr);

        /*vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 5, 1, &renderinfo.simpleShadowMap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 6, 1, &renderinfo.cascadeShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 7, 1, &renderinfo.cascadeLightSpaceMx, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 8, 1, &renderinfo.vsmShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 9, 1, &renderinfo.esmShadowmap, 0, nullptr);*/


        for (int i = 0; i < renderinfo.gameobjects.size(); i++) {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &renderinfo.gameobjects.at(i)->getDescriptorSet(currentFrame), 0, nullptr);
            renderinfo.gameobjects.at(i)->model->draw(cmd, pipelineLayout, currentFrame, false, 1);
        }

       


    }
}