
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


            /*vertexinput*/
            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = nullptr;
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
            configinfo.vertexInputInfo.pVertexBindingDescriptions = nullptr;


            pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

            });
	}

	void SkyRenderSystem::drawSky(VkCommandBuffer& cmd, int currentFrame, FramebufferResources& depthbuffer) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());
        
       
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &depthbuffer.res.descriptorSets[currentFrame], 0, nullptr);

        vkCmdDraw(cmd, 3, 1, 0, 0);


	}
}