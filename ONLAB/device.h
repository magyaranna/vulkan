#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <iostream>
#include <optional>


#include "instance.h"


namespace v {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Device {
	private:

		const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		Instance& instance;

		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkCommandPool commandPool;

	private:

		//void setupDebugMessenger();
		void pickPhysicalDevice();
		VkSampleCountFlagBits getMaxUsableSampleCount();
		void createLogicalDevice();
		void createCommandPool();

		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);


		//swapchain
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


	public:
		Device(Instance& instance);
		~Device();
		Device(const Device&) = delete;
		void operator=(const Device&) = delete;

		SwapChainSupportDetails getSwapChainSupport() {
			return querySwapChainSupport(physicalDevice);
		}
		QueueFamilyIndices findQueueFamilies() {
			return findQueueFamilies(physicalDevice);
		}
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


		/**/
		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		VkDevice getLogicalDevice() { return logicalDevice; }
		VkQueue getGraphicsQueue() { return graphicsQueue; }
		VkQueue getPresentQueue() { return presentQueue; }
		VkCommandPool getCommandPool() { return commandPool; }
		VkSampleCountFlagBits getMSAASampleCountFlag() { return msaaSamples; }

	};


}