
#include "camera.h"


namespace v {

	Camera::Camera(Device& device, SwapChain& swapChain, int w, int h, glm::vec3 p, VkDescriptorSetLayout layout, VkDescriptorPool pool) : device{ device }, swapChain{ swapChain }
	{
		width = w;
		height = h;
		pos = p;

		createGlobalUniformBuffers();
		createGlobalDescriptorSets(layout, pool);
	}

	Camera::~Camera() {
		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.getLogicalDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.getLogicalDevice(), uniformBuffersMemory[i], nullptr);
		}
	}

	void Camera::updateGlobalUniformBuffer(uint32_t currentFrame) {

		GlobalUniformBufferObject ubo{};

		//pos = pos + glm::vec3(5, 5, 5);

		ubo.view = glm::lookAt(pos, pos + oriantation, up);

		ubo.proj = glm::perspective(glm::radians(90.0f), swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height, znear, zfar);
		ubo.proj[1][1] *= -1;



		matrices = ubo;

		void* data;
		vkMapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device.getLogicalDevice(), uniformBuffersMemory[currentFrame]);

	}


	void Camera::Inputs(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pos += speed * oriantation;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			pos += speed * -glm::normalize(glm::cross(oriantation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			pos += speed * -oriantation;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			pos += speed * glm::normalize(glm::cross(oriantation, up));
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			pos += speed * up;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			pos += speed * -up;
		}




		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstClick)
			{
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double mouseX;
			double mouseY;

			glfwGetCursorPos(window, &mouseX, &mouseY);

			float alfaX = 100.0 * (float)(mouseY - (height / 2)) / height;
			float alfaY = 100.0 * (float)(mouseX - (width / 2)) / width;


			//fuggoleges
			glm::vec3 newOrientation = glm::rotate(oriantation, glm::radians(-alfaX), glm::normalize(glm::cross(oriantation, up)));

			oriantation = newOrientation;
			oriantation = glm::rotate(oriantation, glm::radians(-alfaY), up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			firstClick = true;

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		}


	}


	void Camera::createGlobalUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(GlobalUniformBufferObject);

		uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			Helper::createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		}
	}




	void Camera::createGlobalDescriptorSets(VkDescriptorSetLayout layout, VkDescriptorPool pool) {
		std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferinfo{};
			bufferinfo.buffer = uniformBuffers[i];
			bufferinfo.offset = 0;
			bufferinfo.range = 2 * sizeof(glm::mat4);   //VP


			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferinfo;

			vkUpdateDescriptorSets(device.getLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

}