#pragma once


#include "vulkan/vulkan.h"
#include <vector>
#include <functional>

#include "device.h"

namespace v {

	class PipelineManager {
	private:
		Device& device;
		std::vector<std::function<void()>> vec;

	public:
		PipelineManager(Device& device) : device(device) {};
		~PipelineManager() {};

		void addPipelineCreation(std::function<void()> func) {

			vec.push_back(func);
			func();
		}

		void compile(){
			vkDeviceWaitIdle(device.getLogicalDevice());
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/scene.vert -o shaders/sceneVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/scene.frag -o shaders/sceneFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.vert -o shaders/terrainVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.frag -o shaders/terrainFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.tesc -o shaders/terrainTesc.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.tese -o shaders/terrainTese.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depth.vert -o shaders/depthVert");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depthCascade.vert -o shaders/depthCascadeVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depth.frag -o shaders/depthFrag");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/quad.vert -o shaders/quadVert.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/vsm.frag -o shaders/vsmFrag");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/esm.frag -o shaders/esmFrag");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/vcsm.frag -o shaders/vcsmFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/ecsm.frag -o shaders/ecsmFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/gaussBlur.frag -o shaders/gaussBlurFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/gaussblur_cascade.frag -o shaders/gaussBlurCascadeFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/skybox.frag -o shaders/skyboxFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/skybox.vert -o shaders/skyboxVert.spv");

		}

		void reload() {

			vkDeviceWaitIdle(device.getLogicalDevice());
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/scene.vert -o shaders/sceneVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/scene.frag -o shaders/sceneFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.vert -o shaders/terrainVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.frag -o shaders/terrainFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.tesc -o shaders/terrainTesc.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain.tese -o shaders/terrainTese.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/terrain_regi.vert -o shaders/terrainVert_regi.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depth.vert -o shaders/depthVert");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depthCascade.vert -o shaders/depthCascadeVert.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/depth.frag -o shaders/depthFrag");
			
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/quad.vert -o shaders/quadVert.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/vsm.frag -o shaders/vsmFrag");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/esm.frag -o shaders/esmFrag");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/vcsm.frag -o shaders/vcsmFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/ecsm.frag -o shaders/ecsmFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/gaussBlur.frag -o shaders/gaussBlurFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/gaussblur_cascade.frag -o shaders/gaussBlurCascadeFrag.spv");

			system("%VULKAN_SDK%/Bin/glslc.exe shaders/skybox.frag -o shaders/skyboxFrag.spv");
			system("%VULKAN_SDK%/Bin/glslc.exe shaders/skybox.vert -o shaders/skyboxVert.spv");

			for (auto& it : vec) {
				it();
			}
		}

	};


}


