#pragma once


#include <vector>
#include <array>
#include "vulkan/vulkan.h"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>



#include "swapchain.h"

#include "helper.h"
#include "buffer.h"
#include "descriptors.h"


namespace v {


	class Camera {
	private:

		Device& device;
		SwapChain& swapChain;

		float znear = 0.1f;
		float zfar = 5000.0f; //2000.0f; //100.0f;

		bool firstClick = true;
		int width, height;
		float speed = 3.0f;
		float sensitivity = 100.0f;

		std::vector<std::unique_ptr<Buffer>> VP_uniform;
		void createUniformBuffers();

		std::vector<VkDescriptorSet> descriptorSets;
		void createDescriptorSets(DescriptorSetLayout& layout, DescriptorPool& pool);

	public:
		glm::vec3 pos;
		glm::vec3 orientation;// = glm::vec3(0.0f, -0.5f, -1.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		Camera(Device& device, SwapChain& swapchain, int width, int height, glm::vec3 pos, glm::vec3 o, DescriptorSetLayout& layout, DescriptorPool& pool);
		~Camera();

		struct CameraUniformBufferObject {
			glm::mat4 view;
			glm::mat4 proj;
		};
		CameraUniformBufferObject matrices;

		void Inputs(GLFWwindow* window, bool invert);
		void updateUniformBuffer(uint32_t currentFrame);

		void setPosition(glm::vec3 p) {
			pos = p;
		}
		void setOrientation(glm::vec3 o) {
			orientation = o;
		}
		glm::vec3 getPosition() { return pos; }
		glm::vec3 getOrientation() { return orientation; }
		float getSpeed() { return speed; }
		float getSensitivity() { return sensitivity; }

		VkDescriptorSet& getDescriptorSet(int i) { return descriptorSets[i]; }

		float getNearClip() { return znear; }
		float getFarClip() { return zfar; }
	};
}