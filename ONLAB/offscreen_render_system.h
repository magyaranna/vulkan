#pragma once

#include "device.h"
#include "pipeline.h"
#include "vertex.h"
#include "shadowmaps.h"
#include "renderinfo.h"
#include "timestamp_query.h"
#include "terrain_render_system.h"

#include <cassert>

namespace v {


	class OffScreenRenderSystem {
	private:
		Device& device;

		TerrainRenderSystem& terrainRenderSystem;
		std::unique_ptr<Pipeline> terrainPipeline;
		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		VkPipelineLayout terrainPipelineLayout;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> terrainSetLayouts);
		void createPipeline(VkRenderPass renderPass);

	public:

		OffScreenRenderSystem(Device& device, TerrainRenderSystem& terrainRenderSystem,
			std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> terrainSetLayouts, VkRenderPass renderPass);
		~OffScreenRenderSystem();


		void renderGameObjects(VkCommandBuffer& cmd, int currentFrame, VkRenderPass& renderPass, FramebufferResources& depthBuffer,
			Camera& camera, Terrain& terrain, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, Gui& gui, glm::vec2 viewport);

	};
}