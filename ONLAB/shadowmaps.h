#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <array>
#include "device.h"
#include "helper.h"
#include "swapchain.h"

namespace v {

#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOW_MAP_DIM 5000

	class ShadowMap {
	protected:
		Device& device;
		VkDeviceMemory mem;
		std::vector<VkDescriptorSet> descriptorSets;

		virtual void createFramebufferResources(VkRenderPass renderPass) = 0;

	public:
		static const int dim = SHADOW_MAP_DIM;
		VkImage image;
		VkFramebuffer frameBuffer;
		VkImageView view;
		VkSampler sampler;

		ShadowMap(Device& device) : device(device) {};
		virtual ~ShadowMap() {}

		VkDescriptorSet& getDescriptorSet(int i) {
			return descriptorSets[i];
		}

		static std::vector<VkDescriptorSet> createDescriptorSets(Device& device, uint32_t binding, ShadowMap& shadowmap, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
	};

	


	class DepthShadowMap : public ShadowMap {

		void createFramebufferResources(VkRenderPass renderPass);

	public:
		DepthShadowMap(Device& device, uint32_t binding, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkRenderPass renderPass);
		~DepthShadowMap();
	};

	


	class ColorShadowMap : public ShadowMap {

		void createFramebufferResources(VkRenderPass renderPass);
	public:
		static const int dim = 2000;
		ColorShadowMap(Device& device, uint32_t binding, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkRenderPass renderPass);
		~ColorShadowMap();
	};




	class CascadeShadowMap : public ShadowMap {

		void createFramebufferResources(VkRenderPass renderPass);

	public:

		std::array<VkImageView, SHADOW_MAP_CASCADE_COUNT> views;
		std::array<VkFramebuffer, SHADOW_MAP_CASCADE_COUNT> frameBuffers;

		CascadeShadowMap(Device& device, uint32_t binding, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkRenderPass renderPass);
		~CascadeShadowMap();

	};




	class ColorCascadeShadowMap : public ShadowMap {

		void createFramebufferResources(VkRenderPass renderPass);

	public:
		static const int dim = 2000;

		std::array<VkImageView, SHADOW_MAP_CASCADE_COUNT> views;
		std::array<VkFramebuffer, SHADOW_MAP_CASCADE_COUNT> frameBuffers;

		ColorCascadeShadowMap(Device& device, uint32_t binding, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkRenderPass renderPass);
		~ColorCascadeShadowMap();

	};


}

