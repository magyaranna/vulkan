

#include "water_render_system.h"

namespace v {

	WaterRenderSystem::WaterRenderSystem(Device& device, PipelineManager& manager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts) : device(device), pipelineManager(manager){

        createPipelineLayout(setLayouts);
        createPipeline(renderPass);

	}
	WaterRenderSystem::~WaterRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void WaterRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts; //camera, terrain modelmx, light , shadowmap,,cascadeShadowmap, cascadeuniform
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        std::vector<VkPushConstantRange> ranges;

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(float);

        VkPushConstantRange pushConstantRange2 = {};
        pushConstantRange2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange2.offset = 16;
        pushConstantRange2.size = sizeof(glm::vec3);

        ranges.push_back(pushConstantRange);
        ranges.push_back(pushConstantRange2);

        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
        pipelineLayoutInfo.pushConstantRangeCount = ranges.size();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
	}
	void WaterRenderSystem::createPipeline(VkRenderPass renderPass) {
        pipelineManager.addPipelineCreation([this, renderPass]() {
            const std::string vert = "shaders/waterVert.spv";
            const std::string frag = "shaders/waterFrag.spv";

            ConfigInfo configinfo{};
            Pipeline::defaultPipelineConfigInfo(configinfo);

            configinfo.pipelineLayout = pipelineLayout;
            configinfo.renderPass = renderPass;

            configinfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;  

            configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
            configinfo.multisampling.sampleShadingEnable = VK_TRUE;
            configinfo.multisampling.minSampleShading = .2f;

            VkVertexInputAttributeDescription attrPos{};
            attrPos.binding = 0;
            attrPos.location = 0;
            attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
            attrPos.offset = offsetof(Vertex, pos);

            VkVertexInputAttributeDescription attrUV{};
            attrUV.binding = 0;
            attrUV.location = 1;
            attrUV.format = VK_FORMAT_R32G32_SFLOAT;
            attrUV.offset = offsetof(Vertex, texCoord);

            std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
            attributeDescriptions.push_back(attrPos);
            attributeDescriptions.push_back(attrUV);

            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.bindingDescriptions = Vertex::getBindingDescription();
            configinfo.attributeDescriptions = attributeDescriptions;
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
            configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();

            pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo);

            });
	}

	void WaterRenderSystem::renderWater(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo, Camera& camera, float moveFactor) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

        std::array<float, 1> constants = { moveFactor };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), constants.data());

        std::array<glm::vec3, 1> constants2 = { camera.getPosition()};
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 16, sizeof(glm::vec3), constants2.data());


        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &renderInfo.water->getReflection().descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &renderInfo.water->getRefraction().descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &renderInfo.water->getDudvTextureDescriptorSets()[currentFrame], 0, nullptr);


        renderInfo.water->draw(cmd);

	}
}