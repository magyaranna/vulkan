
#include "terrain_render_system.h"


namespace v {

    TerrainRenderSystem::TerrainRenderSystem(Device& device, PipelineManager& pipelineManager, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts)
        : device(device), pipelineManager(pipelineManager){
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);

    }



    TerrainRenderSystem::~TerrainRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
    }

    void TerrainRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts; 
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        std::vector<VkPushConstantRange> ranges;

        VkPushConstantRange pushConstantRange1 = {};
        pushConstantRange1.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        pushConstantRange1.offset = 0;
        pushConstantRange1.size = sizeof(pushConstantTesc);   //12b

        VkPushConstantRange pushConstantRange2 = {};
        pushConstantRange2.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        pushConstantRange2.offset = 12;
        pushConstantRange2.size = sizeof(pushConstantTese);  //20b

        VkPushConstantRange pushConstantRange3 = {};
        pushConstantRange3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange3.offset = 36;
        pushConstantRange3.size = sizeof(pushConstants);

        ranges.push_back(pushConstantRange1);
        ranges.push_back(pushConstantRange2);
        ranges.push_back(pushConstantRange3);

        pipelineLayoutInfo.pPushConstantRanges = ranges.data();
        pipelineLayoutInfo.pushConstantRangeCount = ranges.size();

        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }

    void TerrainRenderSystem::createPipeline(VkRenderPass renderPass) {

          pipelineManager.addPipelineCreation([this, renderPass]() {
            const std::string vert = "shaders/terrainVert.spv";
            const std::string frag = "shaders/terrainFrag.spv";
            const std::string tesc = "shaders/terrainTesc.spv";
            const std::string tese = "shaders/terrainTese.spv";

            ConfigInfo configinfo{};
            Pipeline::defaultPipelineConfigInfo(configinfo);

            configinfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            configinfo.tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            configinfo.tessellation.patchControlPoints = 4;
            
            configinfo.pipelineLayout = pipelineLayout;
            configinfo.renderPass = renderPass;

            //configinfo.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;     
           //configinfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;           

            configinfo.multisampling.rasterizationSamples = device.getMSAASampleCountFlag();
            configinfo.multisampling.sampleShadingEnable = VK_TRUE;
            configinfo.multisampling.minSampleShading = .2f;



            configinfo.rasterizer.cullMode = VK_CULL_MODE_NONE;
            configinfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            configinfo.depthStencil.depthTestEnable = VK_TRUE;
            configinfo.depthStencil.depthWriteEnable = VK_TRUE;
            configinfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            configinfo.depthStencil.depthBoundsTestEnable = VK_FALSE;
            configinfo.depthStencil.stencilTestEnable = VK_TRUE;

            configinfo.depthStencil.front = {};
            configinfo.depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS; 
            configinfo.depthStencil.front.passOp = VK_STENCIL_OP_REPLACE; 
            configinfo.depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
            configinfo.depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP; 
            configinfo.depthStencil.front.reference = 1;
            configinfo.depthStencil.front.compareMask = 0xff;
            configinfo.depthStencil.front.writeMask = 0xff;
            
            configinfo.depthStencil.back = configinfo.depthStencil.front;



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

            VkVertexInputAttributeDescription attrUV{};
            attrUV.binding = 0;
            attrUV.location = 2;
            attrUV.format = VK_FORMAT_R32G32_SFLOAT;
            attrUV.offset = offsetof(Vertex, texCoord);


            std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
            attributeDescriptions.push_back(attrPos);
            attributeDescriptions.push_back(attrNormal);
            attributeDescriptions.push_back(attrUV);

            configinfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            configinfo.bindingDescriptions = Vertex::getBindingDescription();
            configinfo.attributeDescriptions = attributeDescriptions;
            configinfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
            configinfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configinfo.attributeDescriptions.size());
            configinfo.vertexInputInfo.pVertexBindingDescriptions = &configinfo.bindingDescriptions;
            configinfo.vertexInputInfo.pVertexAttributeDescriptions = configinfo.attributeDescriptions.data();


           
            pipeline = std::make_unique<Pipeline>(device, vert, frag, configinfo, tesc, tese);

            configinfo.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

            pipelineWireFrame = std::make_unique<Pipeline>(device, vert, frag, configinfo, tesc, tese);


           });


    }

    void TerrainRenderSystem::renderTerrain(VkCommandBuffer& cmd, int currentFrame, RenderInfo renderInfo, Camera& camera, glm::vec4 clipPlane) {

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderInfo.gui.wireframe ? pipelineWireFrame->getGraphicsPipeline() : pipeline->getGraphicsPipeline());


        
       
        std::array<pushConstantTesc, 1> constants1 = { {renderInfo.viweport, renderInfo.gui.tessFactor} };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 0, sizeof(pushConstantTesc), constants1.data());

        std::array<pushConstantTese, 1> constants2 = { renderInfo.gui.dFactor, clipPlane };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 12, sizeof(pushConstantTese), constants2.data());


        renderInfo.gui.displayNormalmap == true ? pushConstants[0] = 1 : pushConstants[0] = 0;
        renderInfo.gui.cascade == true ? pushConstants[1] = 1 : pushConstants[1] = 0;
        renderInfo.gui.vsm == true ? pushConstants[2] = 1 : pushConstants[2] = 0;
        renderInfo.gui.esm == true ? pushConstants[3] = 1 : pushConstants[3] = 0;
        renderInfo.gui.cascadecolor == true ? pushConstants[4] = 1 : pushConstants[4] = 0;
        renderInfo.gui.pcf == true ? pushConstants[5] = 1 : pushConstants[5] = 0;
        renderInfo.gui.bias == true ? pushConstants[6] = 1 : pushConstants[6] = 0;
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 36, sizeof(pushConstants), pushConstants.data());


        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &renderInfo.terrain->getGrassTextureDescriptorSets(currentFrame), 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &camera.getDescriptorSet(currentFrame), 0, nullptr);
        //modelmx
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &renderInfo.terrain->getDescriptorSet(currentFrame), 0, nullptr);

        
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &renderInfo.terrain->getHeightMapDescriptorSet(currentFrame), 0, nullptr);


        
       /* vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &renderInfo.cascadeShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 5, 1, &renderInfo.cascadeLightSpaceMx, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 6, 1, &renderInfo.vsmShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 7, 1, &renderInfo.esmShadowmap, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 8, 1, &renderInfo.simpleShadowMap, 0, nullptr);
        */

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &renderInfo.terrain->getNormalMapDescriptorSet(currentFrame), 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 5, 1, &renderInfo.light->getLightDescriptorSet(currentFrame), 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 6, 1, &renderInfo.terrain->getSnowTextureDescriptorSets(currentFrame), 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 7, 1, &renderInfo.terrain->getSandTextureDescriptorSets(currentFrame), 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 8, 1, &renderInfo.terrain->getRockTextureDescriptorSets(currentFrame), 0, nullptr);


        renderInfo.terrain->draw(cmd);
    }

}