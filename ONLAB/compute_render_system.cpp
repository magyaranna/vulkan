
#include "compute_render_system.h"


namespace v {

	ComputeRenderSystem::ComputeRenderSystem(Device& device, PipelineManager& pipelineManager, std::vector<VkDescriptorSetLayout> setLayouts) : device(device), pipelineManager (pipelineManager) {

        createPipelineLayouts({ setLayouts[0], setLayouts[1] }, transmittancePipelineLayout);
        createPipelineLayouts({ setLayouts[0], setLayouts[1]}, multiscatteringPipelineLayout);
        createPipelineLayouts(setLayouts, skyViewPipelineLayout);

        createPipeline(transmittancePipeline, transmittancePipelineLayout, "shaders/transmittanceComp.spv");
        createPipeline(multiscatteringPipeline, multiscatteringPipelineLayout, "shaders/multiscatteringComp.spv");
        createPipeline(skyViewPipeline, skyViewPipelineLayout, "shaders/skyviewComp.spv");
	}
	ComputeRenderSystem::~ComputeRenderSystem() {
        vkDestroyPipelineLayout(device.getLogicalDevice(), transmittancePipelineLayout, nullptr);
        vkDestroyPipeline(device.getLogicalDevice(), transmittancePipeline, nullptr);

        vkDestroyPipelineLayout(device.getLogicalDevice(), multiscatteringPipelineLayout, nullptr);
        vkDestroyPipeline(device.getLogicalDevice(), multiscatteringPipeline, nullptr);

        vkDestroyPipelineLayout(device.getLogicalDevice(), skyViewPipelineLayout, nullptr);
        vkDestroyPipeline(device.getLogicalDevice(), skyViewPipeline, nullptr);
	}

	void ComputeRenderSystem::createPipelineLayouts(std::vector<VkDescriptorSetLayout> setLayouts, VkPipelineLayout& pipelineLayout) {
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        std::vector<VkDescriptorSetLayout> layouts = setLayouts;  
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();


        if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
	}

	void ComputeRenderSystem::createPipeline(VkPipeline& pipeline, VkPipelineLayout pipelineLayout, const std::string& filename) {

        pipelineManager.addPipelineCreation([this, &pipeline, pipelineLayout, filename]() {
            auto computeShaderCode = Pipeline::readFile(filename);

            VkShaderModule computeShaderModule = Pipeline::createShaderModule(device, computeShaderCode);

            VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
            computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageInfo.module = computeShaderModule;
            computeShaderStageInfo.pName = "main";

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.stage = computeShaderStageInfo;

            if (vkCreateComputePipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute pipeline!");
            }

            vkDestroyShaderModule(device.getLogicalDevice(), computeShaderModule, nullptr);

            });
	}

    void ComputeRenderSystem::recordComputeCommandBuffers(VkCommandBuffer& cmd, int currentFrame, Sky& sky, Gui& gui, Camera& camera) {
      
        /*transmittance*/
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, transmittancePipeline);

        //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, transmittancePipelineLayout, 0, 1, &sky.inputImage.descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, transmittancePipelineLayout, 0, 1, &sky.transmittanceLUT.descriptorSets[currentFrame], 0, nullptr);

        vkCmdDispatch(cmd, sky.transmittanceLUT.width / 8, sky.transmittanceLUT.height / 4, 1);
           
        /*VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &memoryBarrier,
            0, nullptr,
            0, nullptr
        );*/
      

        /*multiscattering*/
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, multiscatteringPipeline);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, multiscatteringPipelineLayout, 0, 1, &sky.transmittanceLUT.descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, multiscatteringPipelineLayout, 1, 1, &sky.multiscatteringLUT.descriptorSets[currentFrame], 0, nullptr);

        vkCmdDispatch(cmd, sky.multiscatteringLUT.width/8, sky.multiscatteringLUT.height/4, 1);


        /*vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &memoryBarrier,
            0, nullptr,
            0, nullptr
        );*/
       

        /*skyview*/
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, skyViewPipeline);

       // std::array<glm::vec3, 1> constant = { glm::vec3{0.5f} };

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, skyViewPipelineLayout, 0, 1, &sky.transmittanceLUT.descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, skyViewPipelineLayout, 1, 1, &sky.multiscatteringLUT.descriptorSets[currentFrame], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, skyViewPipelineLayout, 2, 1, &sky.skyviewLUT.descriptorSets[currentFrame], 0, nullptr);

        vkCmdDispatch(cmd, sky.skyviewLUT.width / 8, sky.skyviewLUT.height / 4, 1);

        /*vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 1,
            &memoryBarrier,
            0, nullptr,
            0, nullptr
        );
        */
    }
}