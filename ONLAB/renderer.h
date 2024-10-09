#pragma once

#include "swapchain.h"

#include "light.h"
#include <cassert>
#include <array>

namespace v {


	class Renderer {
	private:

		Window& window;
		Device& device;
		Instance& instance;

		int currentFrameIndex = 0;

		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<VkCommandBuffer> computeCommandBuffers;

		uint32_t currentImageIndex;


		void createCommandBuffers();

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "frame is on progress");
			return commandBuffers[currentFrameIndex];
		}

		VkCommandBuffer getCurrentComputeCommandBuffer() const {
			return computeCommandBuffers[currentFrameIndex];
		}

		void createColorRenderPass();
		void createDepthRenderPass();
		void createNormalmapRenderPass();

	public:

		Renderer(Window& window, Device& device, Instance& instance);
		~Renderer();

		bool isFrameStarted = false;
		
		std::unique_ptr<SwapChain> swapchain;
		VkRenderPass colorRenderPass;
		VkRenderPass depthRenderPass;
		VkRenderPass normalRenderPass;

		VkCommandBuffer beginFrame();
		void endFrame();

		VkCommandBuffer beginCompute();
		void endCompute();

		
		void beginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer fb = VK_NULL_HANDLE, VkRenderPass renderPass = VK_NULL_HANDLE);

		void endRenderPass(VkCommandBuffer commandBuffer);

		int getFrameIndex() { return currentFrameIndex; }

	};
}