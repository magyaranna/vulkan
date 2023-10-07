
#include "app.h"


namespace v {
    

    App::App() {
        glfwSetWindowUserPointer(window.getWindow(), this);
        glfwSetFramebufferSizeCallback(window.getWindow(), framebufferResizeCallback);

    }

    App::~App() { 

    }

    void App::run() {

        gui = std::make_unique<Gui>(window, device, *renderer.swapchain, descriptorPool.getDescriptorPool());


        /*model*/
        auto model_bindings = Binding()
            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)  //texture
            .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); //normalmap
           
        std::unique_ptr<DescriptorSetLayout> model_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, model_bindings.bindings);

        std::shared_ptr<Model> sponza_model = std::make_shared<Model>(device, "models/tree.obj", model_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        /*gameobj*/
        auto gameobject_bindings = Binding()
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);  //model mx
        std::unique_ptr<DescriptorSetLayout> gameobject_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, gameobject_bindings.bindings);
                                                                                                                   
        //std::unique_ptr<GameObject> obj = std::make_unique<GameObject>(0, device, glm::vec3(0.02f, 0.02f, 0.02f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::unique_ptr<GameObject> obj = std::make_unique<GameObject>(0, device, glm::vec3(2.0f, 2.0f, 2.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());

        obj->model = sponza_model;

        gameobjects.emplace(obj->getId(), std::move(obj));


        /*terrain*/
        terrain = std::make_unique<Terrain>(device, glm::vec3(0.5f, 0.5f, 0.5f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        /*light*/
        Binding light_binding = Binding()
            .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .addBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> light_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, light_binding.bindings);

        light = std::make_unique<Light>(device, light_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        /*shadowmap*/
        Binding shadowmap_binding = Binding()
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> shadow_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, shadowmap_binding.bindings);

        

        /*camera*/
        Binding camera_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
        std::unique_ptr<DescriptorSetLayout> camera_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, camera_binding.bindings);

        camera = std::make_unique<Camera>(device, *renderer.swapchain, window.getWidth(),
            window.getHeight(), glm::vec3(0.0f, 10.0f, 10.0f), camera_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        

        /*CascadeShadowMAp Layouts*/
        Binding cascadeShadowmap_binding = Binding()
            .addBinding(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> cascadeShadowmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, cascadeShadowmap_binding.bindings);
        
        Binding lightspacemx_binding = Binding()
            .addBinding(8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> cascadeUniform_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, lightspacemx_binding.bindings);


        Binding vsmShadowMap_Binding = Binding()
            .addBinding(9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> vsmShadowmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, vsmShadowMap_Binding.bindings);


        descriptorLayouts.push_back(camera_descriptorLayout->getDescriptorSetLayout());   //VP
        descriptorLayouts.push_back(model_descriptorLayout->getDescriptorSetLayout());    //Tesxtures
        descriptorLayouts.push_back(gameobject_descriptorLayout->getDescriptorSetLayout()); //M
        descriptorLayouts.push_back(light_descriptorLayout->getDescriptorSetLayout());      //light vp, pos dir
        descriptorLayouts.push_back(shadow_descriptorLayout->getDescriptorSetLayout());  //shadowmap
        descriptorLayouts.push_back(cascadeShadowmap_descriptorLayout->getDescriptorSetLayout());  //cascadeShadowmap
        descriptorLayouts.push_back(cascadeUniform_descriptorLayout->getDescriptorSetLayout());  //cascadeuniform->mx, depth
        descriptorLayouts.push_back(vsmShadowmap_descriptorLayout->getDescriptorSetLayout());  //vsm shadowmap
        
        

        RenderSystem renderSystem{ device, renderer.swapchain->getRenderPass(),descriptorLayouts };
        OffScreenRenderSystem offscreenRenderSystem{ device, { descriptorLayouts[2], descriptorLayouts[3] },shadow_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool()};
        TerrainRenderSystem terrainRenderSystem{ device,renderer.swapchain->getRenderPass(), {descriptorLayouts[0], descriptorLayouts[2], descriptorLayouts[3], descriptorLayouts[4], descriptorLayouts[5], descriptorLayouts[6],  descriptorLayouts[7]} };

        CascadeShadowRenderSystem cascadeRenderSystem{ device, {descriptorLayouts[6], descriptorLayouts[2]},
            descriptorLayouts[5], descriptorPool.getDescriptorPool(), light->getPos() };
        VSM_RenderSystem vsmRenderSystem{ device,{ descriptorLayouts[2], descriptorLayouts[3] } ,vsmShadowmap_descriptorLayout->getDescriptorSetLayout() ,descriptorPool.getDescriptorPool() };

        while (!window.shouldClose()) {
            glfwPollEvents();

            bool isImGuiWindowHovered = gui->isHovered();

            if (!isImGuiWindowHovered) {
                camera->Inputs(window.getWindow());
            }

            glfwPollEvents();

            gui->updateGui(*light.get());

            if (auto commandBuffer = renderer.beginFrame()) {       //acquireNextImage + begincmd

                //update uniforms
                gameobjects.at(0)->updateUniformBuffer(renderer.currentFrameIndex, gui->spin);
                light->updateLightUniformBuffer(renderer.currentFrameIndex);
                camera->updateGlobalUniformBuffer(renderer.currentFrameIndex);
                
                light->updateLightVPUniformBuffer(renderer.currentFrameIndex);
                terrain->updateUniformBuffer(renderer.currentFrameIndex,gui->spin);

                cascadeRenderSystem.updateCascades(camera, light->getDir());
                cascadeRenderSystem.updateUniformBuffers(renderer.currentFrameIndex);

                vkDeviceWaitIdle(device.getLogicalDevice());

                
                cascadeRenderSystem.renderGameObjects(commandBuffer, gameobjects, terrain, renderer.currentFrameIndex);


                offscreenRenderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, gui->vsm, light, gameobjects, terrain);
                vsmRenderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, light, gameobjects, terrain);
                

                renderer.beginSwapChainRenderPass(commandBuffer);
                {

                    RenderInfo renderInfo{
                        camera, light, *gui, terrain, gameobjects,
                        offscreenRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                        cascadeRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                        cascadeRenderSystem.getMXDescriptorSet(renderer.currentFrameIndex),
                        //cascadeRenderSystem.getMXDescriptorSet(renderer.currentFrameIndex),
                        vsmRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex)
                    };

                      terrainRenderSystem.renderTerrain(commandBuffer, renderer.currentFrameIndex,renderInfo);

                      renderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, renderInfo);
                }
                renderer.endRenderPass(commandBuffer);
                

                renderer.endFrame();     //submit
            }                                                    
        }
        vkDeviceWaitIdle(device.getLogicalDevice());
    }


    void App::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->renderer.swapchain->setFramebufferResized(true);

    }
}

