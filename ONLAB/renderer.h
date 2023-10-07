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


		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		

		void createCommandBuffers();

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "frame is on progress");
			return commandBuffers[currentFrameIndex];
		}

	public:

		Renderer(Window& window, Device& device, Instance& instance);
		~Renderer();

		bool isFrameStarted = false;
		int currentFrameIndex =  0 ;
		std::unique_ptr<SwapChain> swapchain;


		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		
		void endRenderPass(VkCommandBuffer commandBuffer);

	};
}