
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
        //std::shared_ptr<Model> m2 = std::make_shared<Model>(device, "models/viking_room.obj", model_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        /*gameobj*/
        auto gameobject_bindings = Binding()
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);  //model mx
        std::unique_ptr<DescriptorSetLayout> gameobject_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, gameobject_bindings.bindings);

        //std::unique_ptr<GameObject> obj = std::make_unique<GameObject>(0, device, glm::vec3(0.02f, 0.02f, 0.02f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::unique_ptr<GameObject> obj1 = std::make_unique<GameObject>(0, device, glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, -3.0f, 0.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        //std::unique_ptr<GameObject> obj2 = std::make_unique<GameObject>(0, device, glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(10.0f, 2.0f, 2.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());

        obj1->model = sponza_model;
        // obj2->model = m2;

         //gameobjects.emplace(obj2->getId(), std::move(obj2));
        gameobjects.emplace(obj1->getId(), std::move(obj1));



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

        /*VSM*/
        Binding vsmShadowMap_Binding = Binding()
            .addBinding(9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> vsmShadowmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, vsmShadowMap_Binding.bindings);

        /*ESM*/
        Binding esmShadowMap_Binding = Binding()
            .addBinding(10, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> esmShadowmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, esmShadowMap_Binding.bindings);

        /*Blured VSM*/
        Binding bluredVSM_Binding = Binding()
            .addBinding(11, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> bluredVsm_descriptorLayouts = std::make_unique<DescriptorSetLayout>(device, bluredVSM_Binding.bindings);


        descriptorLayouts.push_back(camera_descriptorLayout->getDescriptorSetLayout());   //VP
        descriptorLayouts.push_back(model_descriptorLayout->getDescriptorSetLayout());    //Tesxtures
        descriptorLayouts.push_back(gameobject_descriptorLayout->getDescriptorSetLayout()); //M
        descriptorLayouts.push_back(light_descriptorLayout->getDescriptorSetLayout());      //light vp, pos dir
        descriptorLayouts.push_back(shadow_descriptorLayout->getDescriptorSetLayout());  //shadowmap
        descriptorLayouts.push_back(cascadeShadowmap_descriptorLayout->getDescriptorSetLayout());  //cascadeShadowmap
        descriptorLayouts.push_back(cascadeUniform_descriptorLayout->getDescriptorSetLayout());  //cascadeuniform->mx, depth
        // descriptorLayouts.push_back(vsmShadowmap_descriptorLayout->getDescriptorSetLayout());  //vsm shadowmap
        descriptorLayouts.push_back(bluredVsm_descriptorLayouts->getDescriptorSetLayout());  // blured vsm
        descriptorLayouts.push_back(esmShadowmap_descriptorLayout->getDescriptorSetLayout());  //esm shadowmap



        RenderSystem renderSystem{ device, renderer.swapchain->getRenderPass(),descriptorLayouts };
        OffScreenRenderSystem offscreenRenderSystem{ device, { descriptorLayouts[2], descriptorLayouts[3] },shadow_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool() };
        TerrainRenderSystem terrainRenderSystem{ device,renderer.swapchain->getRenderPass(),
            {descriptorLayouts[0], descriptorLayouts[2], descriptorLayouts[3], descriptorLayouts[4], descriptorLayouts[5], descriptorLayouts[6],  descriptorLayouts[7], descriptorLayouts[8]} };

        CascadeShadowRenderSystem cascadeRenderSystem{ device, {descriptorLayouts[6], descriptorLayouts[2]},
            descriptorLayouts[5], descriptorPool.getDescriptorPool(), light->getPos() };

        VSM_RenderSystem vsmRenderSystem{ device,{ descriptorLayouts[2], descriptorLayouts[3] } ,vsmShadowmap_descriptorLayout->getDescriptorSetLayout() ,descriptorPool.getDescriptorPool() };
        ESM_RenderSystem esmRenderSystem{ device,{ descriptorLayouts[4] } ,esmShadowmap_descriptorLayout->getDescriptorSetLayout() ,descriptorPool.getDescriptorPool() };

        BlurSystem blurSystem{ device, {vsmShadowmap_descriptorLayout->getDescriptorSetLayout()}, vsmShadowmap_descriptorLayout->getDescriptorSetLayout(), bluredVsm_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool() };


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
                terrain->updateUniformBuffer(renderer.currentFrameIndex, gui->spin);

                cascadeRenderSystem.updateCascades(camera, light->getDir());
                cascadeRenderSystem.updateUniformBuffers(renderer.currentFrameIndex);

                vkDeviceWaitIdle(device.getLogicalDevice());



                offscreenRenderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, gui->peterPanning, light, gameobjects, terrain);

                cascadeRenderSystem.renderGameObjects(commandBuffer, gameobjects, terrain, renderer.currentFrameIndex);


                vsmRenderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, light, gameobjects, terrain);

                //Helper::transitionImageLayout(device, vsmRenderSystem.getShadowMapVSM().colorImage, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vsmRenderSystem.getShadowMapVSM().mipLevels);
                //Helper::generateMipmaps(device, vsmRenderSystem.getShadowMapVSM().colorImage, VK_FORMAT_R32G32_SFLOAT, SHADOWMAP_DIM, SHADOWMAP_DIM, vsmRenderSystem.getShadowMapVSM().mipLevels);

                blurSystem.render(commandBuffer, vsmRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex), renderer.currentFrameIndex);

                esmRenderSystem.renderGameObjects(commandBuffer, renderer.currentFrameIndex, light, gameobjects, terrain, offscreenRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex));


                renderer.beginSwapChainRenderPass(commandBuffer);
                {

                    RenderInfo renderInfo{
                        camera, light, *gui, terrain, gameobjects,
                        offscreenRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                        cascadeRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                        cascadeRenderSystem.getMXDescriptorSet(renderer.currentFrameIndex),
                        blurSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                        // vsmRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex),
                         esmRenderSystem.getShadowmapDescriptorSet(renderer.currentFrameIndex)
                    };

                    terrainRenderSystem.renderTerrain(commandBuffer, renderer.currentFrameIndex, renderInfo);

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

