#pragma once

#include "vulkan/vulkan.h"
#include "pipeline.h"
#include "camera.h"
#include "gameobject.h"
#include "terrain.h"
#include "vertex.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderinfo.h"
#include "shadowmaps.h"
#include "timestamp_query.h"

namespace v {

	struct Cascade {
		float splitDepth;
		glm::mat4 viewProjMx;

	};

#define SHADOW_MAP_CASCADE_COUNT 4
	class CascadeShadowRenderSystem {
	private:

		Device& device;

		VkPipelineLayout pipelineLayout;
		VkPipelineLayout colorPipelineLayout;

		/*simple*/

		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<Pipeline> pipelineVSM;
		std::unique_ptr<Pipeline> pipelineESM;


		std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;		

		std::array<int, 1> pushConstants;
		//float cascadeSplitLambda = 0.2f;

		struct UniformLightSpace {
			std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> cascadeViewProjMat;
			float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
		} ubo;

		std::vector<VkBuffer> uniformbuffer;
		std::vector<VkDeviceMemory> uniformbufferMem;

		std::vector<VkDescriptorSet> MXDescriptorSets;


		void createUniformBuffers();

		void createDescriptorSets(VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool);    //viewprojmx + index

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> setLayoutsESM);
		void createPipeline(VkRenderPass renderPass, VkRenderPass colorRenderPass);


		std::vector<glm::mat4> getLightSpaceMatrices(std::unique_ptr<Camera> const& camera, glm::vec3 dir, std::vector<float> shadowCascadeLevels);
		glm::mat4 getLightSpaceMatrix(std::unique_ptr<Camera> const& camera, glm::vec3 dir, const float nearPlane, const float farPlane);



	public:

		std::unique_ptr<TS_query> ts;


		CascadeShadowRenderSystem(Device& device,
			std::vector<VkDescriptorSetLayout> setLayouts, std::vector<VkDescriptorSetLayout> setLayoutsESM,  VkDescriptorPool pool, VkRenderPass rp, VkRenderPass colorRp);
		~CascadeShadowRenderSystem();


		void updateCascades(std::unique_ptr<Camera> const& camera, glm::vec3 dir, float splitLambda);
		void updateUniformBuffers(uint32_t currentImage);


		void renderGameObjects(OffScreenRenderInfo renderinfo, CascadeShadowMap& shadowMap);
		void renderGameObjects(OffScreenRenderInfo renderinfo, ColorCascadeShadowMap& shadowMap, CascadeShadowMap& depthshadowmap);
		void renderScene(OffScreenRenderInfo renderinfo, int cascadeIndex);
		void renderScene(OffScreenRenderInfo renderinfo, int cascadeIndex, CascadeShadowMap& shadowMap);


		VkDescriptorSet& getMXDescriptorSet(int i) {
			return MXDescriptorSets[i];
		}
	};


}