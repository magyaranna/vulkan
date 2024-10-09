#pragma once


#include <vector>
#include <array>
#include <string>

#include "swapchain.h"
#include "model.h"


namespace v {

	struct ConfigInfo {
		ConfigInfo() = default;
		ConfigInfo(const ConfigInfo&) = delete;
		ConfigInfo& operator=(const ConfigInfo&) = delete;

		VkVertexInputBindingDescription bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		//std::vector<VkPipelineShaderStageCreateInfo> shaderStages;   

		VkPipelineVertexInputStateCreateInfo vertexInputInfo;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly;

		VkPipelineTessellationStateCreateInfo tessellation;

		VkPipelineViewportStateCreateInfo viewportState;

		std::vector<VkDynamicState> dynamicStates;
		VkPipelineDynamicStateCreateInfo dynamicState;


		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineMultisampleStateCreateInfo multisampling;

		VkPipelineDepthStencilStateCreateInfo depthStencil;

		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlending;
		
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		
	};


	class Pipeline {
	private:

		Device& device;
		VkPipeline pipeline;

		void createGraphicsPipeline(const std::string& vert, const std::string& frag,  ConfigInfo& configInfo, const std::string& tesc, const std::string& tese);
		void createComputePipeline();

	public:
		
		Pipeline(Device& device, const std::string& vert, const std::string& frag,  ConfigInfo& configInfo, const std::string& tesc = "", const std::string& tese = "");
		~Pipeline();
		Pipeline(const Pipeline&) = delete;
		void operator=(const Pipeline&) = delete;

		static VkShaderModule createShaderModule(Device& device, const std::vector<char>& code);
		static std::vector<char> readFile(const std::string& filename);

		static void defaultPipelineConfigInfo(ConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);

	
		VkPipeline getGraphicsPipeline() { return pipeline; }
		
	};
}