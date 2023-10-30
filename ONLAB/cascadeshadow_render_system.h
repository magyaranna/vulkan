#pragma once

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include "camera.h"
#include "gameobject.h"
#include "terrain.h"
#include "vertex.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace v {

	struct CascadeShadowmap {
		//imaeg, mem, imageview, sampler
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkSampler sampler;

		std::vector<VkDescriptorSet> descriptorSets;


	};

	struct Cascade {
		VkImageView view;
		VkFramebuffer frameBuffer;

		float splitDepth;
		glm::mat4 viewProjMx;

	};

#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOWMAP_DIM 3000

	class CascadeShadowRenderSystem {
	private:

		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Pipeline> pipeline;
		VkRenderPass renderPass;

		float cascadeSplitLambda = 0.2f;
		CascadeShadowmap shadowMap;
		std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;

		std::array<int, 1> pushConstants;

		struct UniformLightSpace {
			std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> cascadeViewProjMat;
			float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
		} ubo;

		std::vector<VkBuffer> uniformbuffer;
		std::vector<VkDeviceMemory> uniformbufferMem;


		void createUniformBuffers();

		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);    //viewprojmx + index
		std::vector<VkDescriptorSet> MXDescriptorSets;


		void createShadowMapDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool, VkDevice device);    //shadowmap

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
		void createPipeline();


		void createRenderPass();
		void createShadowmapResources();

		std::vector<glm::mat4> getLightSpaceMatrices(std::unique_ptr<Camera> const& camera, glm::vec3 dir, std::vector<float> shadowCascadeLevels);
		glm::mat4 getLightSpaceMatrix(std::unique_ptr<Camera> const& camera, glm::vec3 dir, const float nearPlane, const float farPlane);



	public:

		CascadeShadowRenderSystem(Device& device,
			std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSetLayout shadowmapLayout, VkDescriptorPool descriptorPool, glm::vec4 lightPos);
		~CascadeShadowRenderSystem();


		void updateCascades(std::unique_ptr<Camera> const& camera, glm::vec3 dir);
		void updateUniformBuffers(uint32_t currentImage);

		void renderGameObjects(VkCommandBuffer commandBuffer, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain,
			int currentFrame);
		void renderScene(VkCommandBuffer commandBuffer, std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects, std::unique_ptr<Terrain> const& terrain
			, uint32_t cascadeIndex, int currentFrame);



		VkDescriptorSet& getShadowmapDescriptorSet(int i) {
			return shadowMap.descriptorSets[i];
		}
		VkDescriptorSet& getMXDescriptorSet(int i) {
			return MXDescriptorSets[i];
		}
	};


}