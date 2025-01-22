
#include "sky_render_system.h"

namespace v {

    SkyRenderSystem::SkyRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts)
        : device(device), manager(manager) {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);

    }
    SkyRenderSystem::~SkyRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayoutForWater, nullptr);
    }


    void SkyRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkPushConstantRange> ranges;

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(v::ComputeRenderSystem::pushConstantCompute);
        ranges.push_back(pushConstantRange);

        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
        pipelineLayoutInfo.pushConstantRangeCount = ranges.size();


        std::vector<VkDescriptorSetLayout> layouts = setLayouts;
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        /**/

        VkPipelineLayoutCreateInfo pipelineLayoutInfoForWater{};
        pipelineLayoutInfoForWater.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkPushConstantRange> ranges2;

        VkPushConstantRange pushConstantRange1 = {};
        pushConstantRange1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange1.offset = 0;
        pushConstantRange1.size = sizeof(v::ComputeRenderSystem::pushConstantCompute);
        ranges2.push_back(pushConstantRange1);

        VkPushConstantRange pushConstantRange2 = {};
        pushConstantRange2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange2.offset = 32;
        pushConstantRange2.size = sizeof(glm::vec4);

        ranges2.push_back(pushConstantRange2);

        pipelineLayoutInfoForWater.pPushConstantRanges = ranges2.data();
        pipelineLayoutInfoForWater.pushConstantRangeCount = ranges2.size();


        std::vector<VkDescriptorSetLayout> layouts2 = setLayouts;
        pipelineLayoutInfoForWater.setLayoutCount = layouts2.size();
        pipelineLayoutInfoForWater.pSetLayouts = layouts2.data();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfoForWater, nullptr, &pipelineLayoutForWater) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        /**/
        
    }
    void SkyRenderSystem::createPipeline(VkRenderPass renderPass) {
        manager.addPipelineCreation([this, renderPass]() {
            const std::string vert = "shaders/quadvert.spv";
            const std::string frag = "shaders/skyFrag.spv";

            ConfigInfo configinfo{};
            Pipeline::defaultPipelineConfigInfo(configinfo);

            configinfo.pipelineLayout = pipelineLayout;
            configinfo.renderPass = renderPass;

            configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            configinfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
            configinfo.multisampling.sampleShadingEnable = VK_TRUE;
            configinfo.multisampling.minSampleShading = .2f;

            configinfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            configinfo.depthStencil.depthTestEnable = VK_FALSE;
            configinfo.depthStencil.depthWriteEnable = VK_FALSE;
            configinfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            configinfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
            configinfo.depthStencil.stencilTestEnable = VK_TRUE;


            configinfo.depthStencil.front = {};
            configinfo.depthStencil.front.compareOp = VK_COMPARE_OP_EQUAL;;
            configinfo.depthStencil.front.passOp = VK_STENCIL_OP_KEEP;
            configinfo.depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
            configinfo.depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
            configinfo.depthStencil.front.reference = 0;
            configinfo.depthStencil.front.compareMask = 0xff;
            configinfo.depthStencil.front.writeMask = 0xff;

            configinfo.depthStencil.back = configinfo.depthStencil.front;

            /*vertexinput*/
            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;


            pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

            });
        manager.addPipelineCreation([this, renderPass]() {
            const std::string vert = "shaders/fullScreenTriangleVert.spv";
            const std::string frag = "shaders/skyFrag.spv";

            ConfigInfo configinfo{};
            Pipeline::defaultPipelineConfigInfo(configinfo);

            configinfo.pipelineLayout = pipelineLayoutForWater;
            configinfo.renderPass = renderPass;

            configinfo.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            configinfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
            configinfo.multisampling.sampleShadingEnable = VK_TRUE;
            configinfo.multisampling.minSampleShading = .2f;

            
            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;


            pipelineForWater = std::make_unique<Pipeline>(device, vert, frag, configinfo);

            });
    }

    void SkyRenderSystem::drawSky(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera, bool water, glm::vec4 clipPlane) {

        water ?
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineForWater->getGraphicsPipeline()) :
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

        glm::vec3 sunDirection = glm::vec3(
            glm::cos(glm::radians(gui.sunPhiAngle)) * glm::cos(glm::radians(gui.sunThetaAngle)),
            glm::sin(glm::radians(gui.sunThetaAngle)),
            glm::sin(glm::radians(gui.sunPhiAngle)) * glm::cos(glm::radians(gui.sunThetaAngle))


        );
        if (water) {
            std::array<v::ComputeRenderSystem::pushConstantCompute, 1> constant = { {camera.getPosition(),0.5, sunDirection, 0.5} };
            vkCmdPushConstants(cmd, pipelineLayoutForWater, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(v::ComputeRenderSystem::pushConstantCompute), constant.data());

            std::array<glm::vec4, 1> constants = { clipPlane };
            vkCmdPushConstants(cmd, pipelineLayoutForWater, VK_SHADER_STAGE_VERTEX_BIT, 32, sizeof(glm::vec4), constants.data());
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForWater, 0, 1, &sky.postComputeDescriptorSets[currentFrame], 0, nullptr);

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForWater, 1, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForWater, 2, 1, &sky.transmittanceLUT.descriptorSets[currentFrame], 0, nullptr);

        }
        else {
            
            std::array<v::ComputeRenderSystem::pushConstantCompute, 1> constant = { {camera.getPosition(),0.5, sunDirection, 0.5} };
            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(v::ComputeRenderSystem::pushConstantCompute), constant.data());

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &sky.postComputeDescriptorSets[currentFrame], 0, nullptr);

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);
          
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &sky.transmittanceLUT.descriptorSets[currentFrame], 0, nullptr);

        }

        

        vkCmdDraw(cmd, 3, 1, 0, 0);


    }
}