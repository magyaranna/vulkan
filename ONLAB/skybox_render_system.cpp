

#include "skybox_render_system.h" 


namespace v{

	SkyboxRenderSystem::SkyboxRenderSystem(Device& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts) : device(device){
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);


	}
	SkyboxRenderSystem::~SkyboxRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}


    void SkyboxRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts;  
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }



    void SkyboxRenderSystem::createPipeline(VkRenderPass renderPass) {

        const std::string vert = "shaders/skyboxVert.spv";
        const std::string frag = "shaders/skyboxFrag.spv";

        ConfigInfo configinfo{};
        Pipeline::defaultPipelineConfigInfo(configinfo);

        configinfo.pipelineLayout = pipelineLayout;
        configinfo.renderPass = renderPass;


        VkVertexInputAttributeDescription attrPos{};
        attrPos.binding = 0;
        attrPos.location = 0;
        attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
        attrPos.offset = offsetof(Vertex, pos);

        std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.push_back(attrPos);

        configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
        configinfo.multisampling.sampleShadingEnable = VK_TRUE;
        configinfo.multisampling.minSampleShading = .2f;

        configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        configinfo.bindingDescriptions = Vertex::getBindingDescription();
        configinfo.attributeDescriptions = attributeDescriptions;
        configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
        configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
        configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
        configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();

        pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

    }



    void SkyboxRenderSystem::drawSkybox(VkCommandBuffer& cmd, int currentFrame, SkyBox& skybox, Camera& camera) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);
        
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &skybox.getDescriptorSet(currentFrame), 0, nullptr);

        skybox.draw(cmd);
    }



}