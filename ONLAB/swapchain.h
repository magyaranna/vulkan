#pragma once

#include "device.h"
#include "helper.h"

#include <vulkan/vulkan.h>

namespace v {

	class SwapChain {
	private:
		Window& window;
		Instance& instance;
		Device& device;


		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;


		VkRenderPass renderPass;

		std::vector<VkFramebuffer> swapChainFramebuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;


		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;


		bool framebufferResized = false;


		void createSwapChain();
		void createImageViews();

		void createRenderPass();


		void createFramebuffers();
		void createSyncObjects();

		void createColorResources();
		void createDepthResources();

		void cleanupSwapChain();


		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);





	public:
		uint32_t currentFrame = 0;
		static const int MAX_FRAMES_IN_FLIGHT = 2;

		SwapChain(Window& window, Device& device, Instance& instance);
		~SwapChain();
		SwapChain(const SwapChain&) = delete;
		void operator=(const SwapChain&) = delete;


		void recreateSwapChain();




		//draw
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffer(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		/**/
		VkSwapchainKHR getSwapChain() { return swapChain; }
		std::vector<VkImage> getSwapChainImages() { return swapChainImages; }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		std::vector<VkImageView> getSwapChainImageViews() { return swapChainImageViews; }
		VkRenderPass getRenderPass() { return renderPass; }

		VkFramebuffer getSwapChainFramebuffers(int i) { return swapChainFramebuffers[i]; }
		std::vector<VkSemaphore> getImageAvailableSemaphores() { return imageAvailableSemaphores; }
		std::vector<VkSemaphore> getRenderFinishedSemaphores() { return renderFinishedSemaphores; }
		std::vector<VkFence> getInFlightFences() { return inFlightFences; }
		VkImage getDepthImage() { return depthImage; }
		VkDeviceMemory getDepthImageMemory() { return depthImageMemory; }
		VkImageView getDepthImageView() { return depthImageView; }
		bool getFramebufferResized() { return framebufferResized; }
		void setFramebufferResized(bool value) { framebufferResized = value; }




	};
}