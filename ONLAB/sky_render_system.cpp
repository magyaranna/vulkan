
#include "sky_render_system.h"

namespace v {

	SkyRenderSystem::SkyRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts)
        : device(device), manager(manager){
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);

	}
	SkyRenderSystem::~SkyRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


	void SkyRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkPushConstantRange> ranges;

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(pushConstantSky);

        ranges.push_back(pushConstantRange);

        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
        pipelineLayoutInfo.pushConstantRangeCount = ranges.size();


        std::vector<VkDescriptorSetLayout> layouts = setLayouts;
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
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
	}

	void SkyRenderSystem::drawSky(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());


        std::array<pushConstantSky, 1> constant = { camera.getPosition()};


        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstantSky), constant.data());

       
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &sky.postComputeDescriptorSets[currentFrame], 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);


        vkCmdDraw(cmd, 3, 1, 0, 0);


	}
}