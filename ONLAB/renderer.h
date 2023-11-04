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

		uint32_t currentImageIndex;


		void createCommandBuffers();

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "frame is on progress");
			return commandBuffers[currentFrameIndex];
		}

		void createColorRenderPass();
		void createDepthRenderPass();

	public:

		Renderer(Window& window, Device& device, Instance& instance);
		~Renderer();

		bool isFrameStarted = false;
		
		std::unique_ptr<SwapChain> swapchain;
		VkRenderPass colorRenderPass;
		VkRenderPass depthRenderPass;


		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void endRenderPass(VkCommandBuffer commandBuffer);

		int getFrameIndex() { return currentFrameIndex; }

	};
}