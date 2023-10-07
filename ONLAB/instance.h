#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include "window.h"


namespace v {



	class Instance {
	private:
		const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
		};
		
		Window& window;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;

		void createInstance();
		void setupDebugMessenger();
		void createSurface();
	

	private:
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
				const VkAllocationCallbacks* pAllocator,
				VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
				VkDebugUtilsMessengerEXT debugMessenger,
				const VkAllocationCallbacks* pAllocator);

	public:

		Instance(Window& window);
		~Instance();
		Instance(const Instance&) = delete;
		void operator=(const Instance&) = delete;

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif


		/**/
		const std::vector<const char*> getValidationLayers() { return validationLayers; }
		VkInstance getInstance() { return instance; }
		VkDebugUtilsMessengerEXT getDebugMessenger() { return debugMessenger; }
		VkSurfaceKHR getSurface() { return surface; }

	};


}