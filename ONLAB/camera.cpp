
#include "camera.h"


namespace v {

	Camera::Camera(Device& device, SwapChain& swapChain, int w, int h, glm::vec3 p, glm::vec3 o, DescriptorSetLayout& layout, DescriptorPool& pool)
		: device{ device }, swapChain{ swapChain }, pos{ p }, orientation{ glm::normalize(o) } {

		width = swapChain.getSwapChainExtent().width;
		height = swapChain.getSwapChainExtent().height;
		createUniformBuffers();
		createDescriptorSets(layout, pool);


	}

	Camera::~Camera() {}

	void Camera::updateUniformBuffer(uint32_t currentFrame) {

		CameraUniformBufferObject ubo{};
		ubo.view = glm::lookAt(pos, pos + orientation, up);
		ubo.proj = glm::perspective(glm::radians(90.0f), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, znear, zfar);
		ubo.proj[1][1] *= -1;
		matrices = ubo;

		void* data;
		vkMapMemory(device.getLogicalDevice(), VP_uniform[currentFrame]->memory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device.getLogicalDevice(), VP_uniform[currentFrame]->memory);

		//VP_uniform[currentFrame]->writeToBuffer(&ubo);
	}


	void Camera::Inputs(GLFWwindow* window, bool invert) {

		width = swapChain.getSwapChainExtent().width;
		height = swapChain.getSwapChainExtent().height;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			pos += speed * orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			pos += speed * -glm::normalize(glm::cross(orientation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			pos += speed * -orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			pos += speed * glm::normalize(glm::cross(orientation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (invert) pos -= speed * up;
			else pos += speed * up;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			if (invert) pos -= speed * -up;
			else pos += speed * -up;
			
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick) {
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double mouseX;
			double mouseY;

			glfwGetCursorPos(window, &mouseX, &mouseY);

			float alfaX = sensitivity * (float)(mouseY - (height / 2)) / height;
			float alfaY = sensitivity * (float)(mouseX - (width / 2)) / width;

			if (invert) {
				alfaX = sensitivity * (float)(mouseY - (height / 2)) / height * (-1);
				alfaY = sensitivity * (float)(mouseX - (width / 2)) / width * (-1);

			}
			glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-alfaX), glm::normalize(glm::cross(orientation, up)));


			if (abs(glm::angle(newOrientation, up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				orientation = newOrientation;
			}
			orientation = glm::rotate(orientation, glm::radians(-alfaY), up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {

			firstClick = true;

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}
	}

	void Camera::createUniformBuffers() {

		VP_uniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDeviceSize bufferSize = sizeof(CameraUniformBufferObject);

		for (int i = 0; i < VP_uniform.size(); i++) {
			VP_uniform[i] = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			//VP_uniform[i]->map();
		}
	}


	void Camera::createDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& pool) {

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < descriptorSets.size(); i++) {
			auto bufferInfo = VP_uniform[i]->descriptorInfo();

			DescriptorWriter(layout, pool)
				.createDescriptorWriter(0, &bufferInfo)
				.build(descriptorSets[i]);
		}
	}

}