
#include "light.h"
#include <stdexcept>
#include <fstream>

namespace v {


	Light::Light(Device& device, DescriptorSetLayout& layout, DescriptorPool& pool) :device{ device } {

		createUniformBuffers();
		createDescriptorSets(layout, pool);

	}
	Light::~Light() {}


	void Light::createUniformBuffers() {

		lightUniform.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		VkDeviceSize bufferSize = sizeof(LightUniformBuffer);

		for (int i = 0; i < lightUniform.size(); i++) {
			lightUniform[i] = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			lightUniform[i]->map();
		}
	}


	void Light::updateUniformBuffer(uint32_t currentImage) {

		LightUniformBuffer ubl = {};
		ubl.lightPosition = position + glm::vec3(dir.x, 0.0, dir.z) * 20.0f;
		ubl.lightDir = dir;

		//ubl.proj = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, 0.1f, 1000.0f);  //50.0f
		ubl.proj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 70.0f);
		//ubl.proj = glm::ortho(-70.0f, 70.0f, -70.0f, 70.0f, 0.1f, 70.0f);


		ubl.view = glm::lookAt(glm::vec3(position.x, position.y, position.z),
			glm::vec3(position.x, position.y, position.z) + dir,
			glm::vec3(0.0f, 1.0f, 0.0f));

		lightUniform[currentImage]->writeToBuffer(&ubl);
	}


	void Light::createDescriptorSets(DescriptorSetLayout& descriptorSetLayout, DescriptorPool& descriptorPool) {

		descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < descriptorSets.size(); i++) {
			auto bufferInfo  = lightUniform[i]->descriptorInfo(sizeof(LightUniformBuffer));

			DescriptorWriter(descriptorSetLayout, descriptorPool)
				.createDescriptorWriter(0, &bufferInfo)
				.build(descriptorSets[i]);
		}
	}




}