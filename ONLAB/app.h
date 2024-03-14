#pragma once

#include "instance.h"
#include "window.h"
#include "device.h"

#include "model.h"
#include "gameobject.h"


#include "light.h"
#include "camera.h"

#include "gui.h"
#include "renderer.h"
#include "render_system.h"
#include "offscreen_render_system.h"
#include "terrain_render_system.h"
#include "cascadeshadow_render_system.h"
#include "vsm_render_system.h"
#include "esm_render_system.h"
#include "blur_render_system.h"

#include "descriptors.h"

#include "terrain.h"

#include "shadowmaps.h"
#include "skybox.h"
#include "skybox_render_system.h"

#include "pipelinemanager.h"

namespace v {


    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;


    class App {
    private:

        Window window{ WIDTH, HEIGHT, "Vulkan" };
        Instance instance{ window };
        Device device{ instance };
        Renderer renderer{ window,device, instance };
        PipelineManager pipelineManager{device};


        DescriptorPool descriptorPool{ device,2 };  
        std::vector<VkDescriptorSetLayout> descriptorLayouts;


        std::unique_ptr<Gui> gui;
        std::unique_ptr<Light> light;
        std::unique_ptr<Camera> camera;

        std::unique_ptr<Terrain> terrain;

        std::unique_ptr<SkyBox> skybox;

        // std::vector<GameObject*> gameobjects = {};
        std::unordered_map<unsigned int, std::unique_ptr<GameObject>> gameobjects;

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);


        bool writeFile = false;

    public:


        App();
        ~App();
        App(const App&) = delete;
        App& operator=(const App&) = delete;


        void run();

    };

}