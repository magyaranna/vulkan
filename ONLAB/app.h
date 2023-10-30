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


namespace v {

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";
    const std::string TEXTURE_PATH_R = "textures/r.png";


    const std::string MODEL_PATH1 = "models/barrel.obj";
    const std::string TEXTURE_PATH1 = "textures/barrel.png";
    const std::string NORMALMAP_PATH1 = "textures/barrelNormal.png";

    const std::string MODEL_PATH2 = "models/sponza.obj";
    const std::string TEXTURE_PATH2 = "textures/r2.png";
    const std::string NORMALMAP_PATH2 = "textures/r.png";



    class App {
    private:

        Window window{ WIDTH, HEIGHT, "Vulkan" };
        Instance instance{ window };
        Device device{ instance };
        Renderer renderer{ window,device, instance };


        DescriptorPool descriptorPool{ device,2 };  ///////?
        std::vector<VkDescriptorSetLayout> descriptorLayouts;


        std::unique_ptr<Gui> gui;
        std::unique_ptr<Light> light;
        std::unique_ptr<Camera> camera;

        std::unique_ptr<Terrain> terrain;



        // std::vector<GameObject*> gameobjects = {};
        std::unordered_map<unsigned int, std::unique_ptr<GameObject>> gameobjects;

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    public:


        App();
        ~App();
        App(const App&) = delete;
        App& operator=(const App&) = delete;


        void run();

    };

}