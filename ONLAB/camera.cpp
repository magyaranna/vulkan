
#include "camera.h"


namespace v {

	Camera::Camera(Device& device, SwapChain& swapChain, int w, int h, glm::vec3 p, DescriptorSetLayout& layout, DescriptorPool& pool)
		: device{ device }, swapChain{ swapChain } , pos{ p} {

		width = swapChain.getSwapChainExtent().width;
		height = swapChain.getSwapChainExtent().height;
		createUniformBuffers();
		createDescriptorSets(layout, pool);

		
	}

	Camera::~Camera() {}

	void Camera::updateUniformBuffer(uint32_t currentFrame) {

		CameraUniformBufferObject ubo{};
		ubo.view = glm::lookAt(pos - oriantation, pos, up);
		ubo.proj = glm::perspective(glm::radians(90.0f), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, znear, zfar);
		ubo.proj[1][1] *= -1;
		matrices = ubo;

		VP_uniform[currentFrame]->writeToBuffer(&ubo);
	}


	void Camera::Inputs(GLFWwindow* window){

		width = swapChain.getSwapChainExtent().width;
		height = swapChain.getSwapChainExtent().height;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
			pos += speed * oriantation;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
			pos += speed * -glm::normalize(glm::cross(oriantation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
			pos += speed * -oriantation;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
			pos += speed * glm::normalize(glm::cross(oriantation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
			pos += speed * up;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
			pos += speed * -up;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick){
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double mouseX;
			double mouseY;

			glfwGetCursorPos(window, &mouseX, &mouseY);

			float alfaX = sensitivity * (float)(mouseY - (height / 2)) / height;  
			float alfaY = sensitivity * (float)(mouseX - (width / 2)) / width;

			//fuggoleges
			glm::vec3 newOrientation = glm::rotate(oriantation, glm::radians(-alfaX), glm::normalize(glm::cross(oriantation, up)));

			// Decides whether or not the next vertical Orientation is legal or not
			if (abs(glm::angle(newOrientation, up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				oriantation = newOrientation;
			}
			oriantation = glm::rotate(oriantation, glm::radians(-alfaY), up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE){

			firstClick = true;

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}
	}

	void Camera::createUniformBuffers() {

		VP_uniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDeviceSize bufferSize = sizeof(CameraUniformBufferObject);

		for (int i = 0; i < VP_uniform.size(); i++) {
			VP_uniform[i] = std::make_unique<Buffer>( device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VP_uniform[i]->map();
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