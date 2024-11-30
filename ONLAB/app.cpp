
#include "app.h"
#include <fstream>


namespace v {


    App::App() {
        glfwSetWindowUserPointer(window.getWindow(), this);
        glfwSetFramebufferSizeCallback(window.getWindow(), framebufferResizeCallback);
        pipelineManager.compile();
    }

    App::~App() {

    }

    void App::run() {

        gui = std::make_unique<Gui>(window, device, *renderer.swapchain, descriptorPool.getDescriptorPool());

        /*camera*/
        Binding camera_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_ALL); //VK_SHADER_STAGE_VERTEX_BIT || VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT || VK_SHADER_STAGE_FRAGMENT_BIT

        std::unique_ptr<DescriptorSetLayout> camera_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, camera_binding.bindings);

        camera = std::make_unique<Camera>(device, *renderer.swapchain, window.getWidth(),
            window.getHeight(), glm::vec3(0.0f, 100.0f, 100.0f), glm::vec3(0.0f, -0.5f, -1.0f), *camera_descriptorLayout, descriptorPool);

        reflectionCamera = std::make_unique<Camera>(device, *renderer.swapchain, window.getWidth(),
            window.getHeight(), camera->getPosition(), camera->getOrientation(), *camera_descriptorLayout, descriptorPool);

        /*texture*/
        auto texture_bindings = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);  //texture
        std::unique_ptr<DescriptorSetLayout> texture_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, texture_bindings.bindings);

        /*normalmap*/
        auto normalmap_bindings = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); //normalmap
        std::unique_ptr<DescriptorSetLayout> normalmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, normalmap_bindings.bindings);

        std::shared_ptr<Model> tree_model = std::make_shared<Model>(device, "models/tree.obj", *texture_descriptorLayout, *normalmap_descriptorLayout, descriptorPool);


        /*gameobj*/
        auto gameobject_bindings = Binding()
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> gameobject_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, gameobject_bindings.bindings);
        std::unique_ptr<GameObject> obj1 = std::make_unique<GameObject>(0, device, glm::vec3(9.0f, 9.0f, 9.0f), glm::vec3(-80.0f, -3.1f, -10.0f), *gameobject_descriptorLayout, descriptorPool);
        obj1->model = tree_model;
        std::unique_ptr<GameObject> obj2 = std::make_unique<GameObject>(0, device, glm::vec3(9.0f, 9.0f, 9.0f), glm::vec3(30.0f, -10.1f, -50.0f), *gameobject_descriptorLayout, descriptorPool);
        obj2->model = tree_model;


        gameobjects.emplace(0, std::move(obj1));
        gameobjects.emplace(1, std::move(obj2));

        /*terrain*/
        terrain = std::make_unique<Terrain>(device, glm::vec3(1.0f, 1.0f, 1.0f), *gameobject_descriptorLayout, *texture_descriptorLayout, descriptorPool, renderer.normalRenderPass);

        /*light*/
        Binding light_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> light_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, light_binding.bindings);
        light = std::make_unique<Light>(device, *light_descriptorLayout, descriptorPool);

        /*skybox*/
        Binding skybox_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> skybox_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, skybox_binding.bindings);
        skybox = std::make_unique<SkyBox>(device, *skybox_descriptorLayout, descriptorPool);

        /*shadowmap*/
        Binding shadowmap_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> shadow_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, shadowmap_binding.bindings);
        DepthShadowMap simpleDepthShadowMap{ device, 0, shadow_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.depthRenderPass };

        /*VSM */
        Binding vsm_Binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> vsm_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, vsm_Binding.bindings);
        ColorShadowMap varianceShadowMap{ device, 0,  vsm_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.colorRenderPass };

        /*ESM*/
        Binding esm_Binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> esm_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, esm_Binding.bindings);
        ColorShadowMap expShadowMap{ device, 0,  esm_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.colorRenderPass };


        /*CSM*/
        Binding csm_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> csm_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, csm_binding.bindings);
        CascadeShadowMap cascadeShadowMap{ device, 0, csm_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.depthRenderPass };

        /*CascadeUniformBuffers*/
        Binding lightspacemx_binding = Binding()
            .addBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> cascadeUniform_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, lightspacemx_binding.bindings);

        /*VCSM*/
        ColorCascadeShadowMap varianceCascadeShadowMap{ device, 0, csm_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.colorRenderPass };

        /*ECSM*/
        ColorCascadeShadowMap expCascadeShadowMap{ device, 0, csm_descriptorLayout->getDescriptorSetLayout(),descriptorPool.getDescriptorPool(), renderer.colorRenderPass };


        /*Blured Images*/
        Binding blur_Binding = Binding()
            .addBinding(20, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        std::unique_ptr<DescriptorSetLayout> blur_descriptorLayouts = std::make_unique<DescriptorSetLayout>(device, blur_Binding.bindings);

        //create descriptors for image to blur
        std::vector<VkDescriptorSet> vsmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, varianceShadowMap, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::vector<VkDescriptorSet> esmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, expShadowMap, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::vector<VkDescriptorSet> vcsmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, varianceCascadeShadowMap, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::vector<VkDescriptorSet> ecsmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, expCascadeShadowMap, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        descriptorLayouts.push_back(camera_descriptorLayout->getDescriptorSetLayout());   //VP
        descriptorLayouts.push_back(texture_descriptorLayout->getDescriptorSetLayout());    //texture
        descriptorLayouts.push_back(normalmap_descriptorLayout->getDescriptorSetLayout());  //normalmap
        descriptorLayouts.push_back(gameobject_descriptorLayout->getDescriptorSetLayout()); //M
        descriptorLayouts.push_back(light_descriptorLayout->getDescriptorSetLayout());      //light vp, pos dir
        descriptorLayouts.push_back(shadow_descriptorLayout->getDescriptorSetLayout());  //shadowmap
        descriptorLayouts.push_back(csm_descriptorLayout->getDescriptorSetLayout());  //cascadeShadowmap
        descriptorLayouts.push_back(cascadeUniform_descriptorLayout->getDescriptorSetLayout());  //cascadeuniform->mx, depth
        descriptorLayouts.push_back(vsm_descriptorLayout->getDescriptorSetLayout());  //vsm shadowmap
        descriptorLayouts.push_back(esm_descriptorLayout->getDescriptorSetLayout());  //esm shadowmap
        


        RenderSystem renderSystem{ device, pipelineManager, renderer.swapchain->getRenderPass(), {
               descriptorLayouts[0], descriptorLayouts[1],  descriptorLayouts[2],descriptorLayouts[3] , descriptorLayouts[4]
        
        } };
        TerrainRenderSystem terrainRenderSystem{ device, pipelineManager, renderer.swapchain->getRenderPass(),
            {descriptorLayouts[1], descriptorLayouts[0],  descriptorLayouts[3],descriptorLayouts[1] , descriptorLayouts[1],
            descriptorLayouts[4] ,descriptorLayouts[1],descriptorLayouts[1],descriptorLayouts[1] } };

        std::vector<VkDescriptorSetLayout> l;
        l.push_back(camera_descriptorLayout->getDescriptorSetLayout());
        l.push_back(skybox_descriptorLayout->getDescriptorSetLayout());
        SkyboxRenderSystem skyboxRenderSystem{ device, renderer.swapchain->getRenderPass(), l };


        ShadowmapRenderSystem shadowmapRenderSystem{ device, {  descriptorLayouts[1],descriptorLayouts[3], descriptorLayouts[4]},  renderer.depthRenderPass };
        VSM_RenderSystem vsmRenderSystem{ device,{ descriptorLayouts[5] },  renderer.colorRenderPass };
        ESM_RenderSystem esmRenderSystem{ device,{ descriptorLayouts[5] } , renderer.colorRenderPass };
        CascadeShadowRenderSystem cascadeRenderSystem{ device, {descriptorLayouts[1], descriptorLayouts[3], descriptorLayouts[7]}, {descriptorLayouts[6]},descriptorPool.getDescriptorPool(),  renderer.depthRenderPass,  renderer.colorRenderPass };
        BlurSystem blurSystem{ device, 20, {blur_descriptorLayouts->getDescriptorSetLayout()}, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool(), renderer.colorRenderPass };


        Normalmap_RenderSystem normalmapSystem{ device, pipelineManager,{ descriptorLayouts[1] }, renderer.normalRenderPass };
        normalmapSystem.make_normalmap(*terrain, renderer.normalRenderPass);

        WaterRenderSystem waterRenderSystem{ device, pipelineManager, renderer.swapchain->getRenderPass(),{descriptorLayouts[0], descriptorLayouts[1], descriptorLayouts[1], descriptorLayouts[1]} };
        water = std::make_unique<Water>(device, renderer.swapchain->getRenderPass(), *texture_descriptorLayout, descriptorPool);

        
        Scene scene{ device, renderer.swapchain->getSwapChainExtent(), renderer.depthRenderPass, *texture_descriptorLayout, descriptorPool};
        /*OffScreenRenderSystem offScreenRenderSystem{device, terrainRenderSystem, {descriptorLayouts[1],descriptorLayouts[3], descriptorLayouts[0]},
            {descriptorLayouts[1],descriptorLayouts[0], descriptorLayouts[3], descriptorLayouts[1]}, renderer.depthRenderPass };
        */

        Binding storageBinding = Binding().addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
        std::unique_ptr<DescriptorSetLayout> storageLayout = std::make_unique<DescriptorSetLayout>(device, storageBinding.bindings);
        
        Sky sky{ device , *storageLayout, *texture_descriptorLayout, descriptorPool };

        ComputeRenderSystem computeRenderSystem{ device, pipelineManager, 
            {storageLayout->getDescriptorSetLayout(), storageLayout->getDescriptorSetLayout() , storageLayout->getDescriptorSetLayout()}
        };

        SkyRenderSystem skyRenderSystem{ device, pipelineManager, renderer.swapchain->getRenderPass() , {descriptorLayouts[1], descriptorLayouts[0]} };
       

        float waterDist = camera->pos.y - water->height;
        reflectionCamera->pos.y = camera->pos.y - 2.0f * waterDist;
        reflectionCamera->orientation.y = camera->orientation.y * (-1.0);

        FrameTimer timer{};
        float moveFactor = 0.0f;
     
        int frameIndex = 0;
        while (!window.shouldClose()) {

            if (gui->clicked & 1) {
                pipelineManager.reload();
                gui->clicked++;
            }
            frameIndex = renderer.getFrameIndex();
            glfwPollEvents();
            bool isImGuiWindowHovered = gui->isHovered();
            if (!isImGuiWindowHovered) {
                Inputs(window.getWindow());
            }
            glfwPollEvents();
            gui->updateGui(*light.get(), *camera.get());



            if (auto computeCmd = renderer.beginCompute()) {


                //update uniforms
                //..

                computeRenderSystem.recordComputeCommandBuffers(computeCmd, renderer.getFrameIndex(), sky, *gui, *camera);

                renderer.endCompute();

            }
            


            if (auto commandBuffer = renderer.beginFrame()) {       //acquireNextImage + begincmd

                //update uniforms
                for (int i = 0; i < gameobjects.size(); i++) {
                    gameobjects.at(i)->updateUniformBuffer(frameIndex, gui->spin);
                }
                light->updateUniformBuffer(frameIndex);
                camera->updateUniformBuffer(frameIndex);
                reflectionCamera->updateUniformBuffer(frameIndex);
                terrain->updateUniformBuffer(frameIndex, gui->spin);
                cascadeRenderSystem.updateCascades(camera, light->getDir(), gui->splitLambda);
                cascadeRenderSystem.updateUniformBuffers(frameIndex);

                vkDeviceWaitIdle(device.getLogicalDevice());
                
                RenderInfo renderInfo{
                       glm::vec2(window.getExtent().height, window.getExtent().width),
                       camera, light, *gui, terrain, gameobjects,
                       simpleDepthShadowMap.getDescriptorSet(frameIndex),
                       nullptr,
                       cascadeRenderSystem.getMXDescriptorSet(frameIndex),
                       varianceShadowMap.getDescriptorSet(frameIndex),
                       expShadowMap.getDescriptorSet(frameIndex),
                       water
                };

                /*water*/
                renderer.beginRenderPass(commandBuffer, water->getReflection().frameBuffer, water->getRenderPass());
                    skyboxRenderSystem.drawSkybox(commandBuffer, frameIndex, *skybox, *reflectionCamera, glm::vec4(0, 1, 0, -water->height));
                    terrainRenderSystem.renderTerrain(commandBuffer, frameIndex, renderInfo, *reflectionCamera, glm::vec4(0, 1, 0, -water->height));
                    //renderSystem.renderGameObjects(commandBuffer, frameIndex, renderInfo, *reflectionCamera, glm::vec4(0, 1, 0, -water->height));
                renderer.endRenderPass(commandBuffer);

                renderer.beginRenderPass(commandBuffer, water->getRefraction().frameBuffer, water->getRenderPass());
                    skyboxRenderSystem.drawSkybox(commandBuffer, frameIndex, *skybox, *camera, glm::vec4(0, -1, 0, water->height));
                    terrainRenderSystem.renderTerrain(commandBuffer, frameIndex, renderInfo, *camera, glm::vec4(0, -1, 0, water->height));
                   // renderSystem.renderGameObjects(commandBuffer, frameIndex, renderInfo, *camera, glm::vec4(0, -1, 0, water->height));
                renderer.endRenderPass(commandBuffer);
                /******/

                /* offScreenRenderSystem.renderGameObjects(commandBuffer, frameIndex, renderer.depthRenderPass, scene.getDepthBuffer(), *camera, *terrain, gameobjects,
                    *gui, glm::vec2(window.getExtent().height, window.getExtent().width));
                */

                moveFactor += 0.03 * timer.getFrameSeconds();

                renderer.beginRenderPass(commandBuffer);
                {

                    if(gui->skybox) skyboxRenderSystem.drawSkybox(commandBuffer, frameIndex, *skybox, *camera);
                    //skyRenderSystem.drawSky(commandBuffer, frameIndex, scene.depthBuffer);

                    terrainRenderSystem.renderTerrain(commandBuffer, frameIndex, renderInfo, *camera);
                    waterRenderSystem.renderWater(commandBuffer, frameIndex, renderInfo, *camera, moveFactor);

                    if(!gui->skybox) skyRenderSystem.drawSky(commandBuffer, frameIndex, sky, *gui, *camera);

                   // renderSystem.renderGameObjects(commandBuffer, frameIndex, renderInfo, *camera);
                    gui->renderGui(commandBuffer);
                    /**/


                    /**/

                   /* skyRenderSystem.drawSky(commandBuffer, frameIndex, scene.depthBuffer);

                    VkClearDepthStencilValue val;
                    val.depth = 1.0f;
                    val.stencil = 0;
                    VkImageSubresourceRange range;
                    range.layerCount = 1;
                    range.levelCount = 1;
                    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    range.baseArrayLayer = 0;
                    range.baseMipLevel = 0;
                    vkCmdClearDepthStencilImage(commandBuffer, renderer.swapchain->depthImage, VK_IMAGE_LAYOUT_GENERAL, &val, 1, &range);

                    renderSystem.renderGameObjects(commandBuffer, frameIndex, renderInfo, *camera);
                    gui->renderGui(commandBuffer);
                    */

                    /**/

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

    void App::Inputs(GLFWwindow* window) {

        float width = renderer.swapchain->getSwapChainExtent().width;
        float height = renderer.swapchain->getSwapChainExtent().height;
        float speed = camera->getSpeed();
        float sensitivity = 100.0f;


        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->pos += speed * camera->orientation;
            reflectionCamera->pos += speed * reflectionCamera->orientation;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->pos += speed * -glm::normalize(glm::cross(camera->orientation, camera->up));
            reflectionCamera->pos += speed * -glm::normalize(glm::cross(reflectionCamera->orientation, reflectionCamera->up));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->pos += speed * -camera->orientation;
            reflectionCamera->pos += speed * -reflectionCamera->orientation;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->pos += speed * glm::normalize(glm::cross(camera->orientation, camera->up));
            reflectionCamera->pos += speed * glm::normalize(glm::cross(reflectionCamera->orientation, reflectionCamera->up));
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera->pos += speed * camera->up;
            reflectionCamera->pos -= speed * reflectionCamera->up;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            camera->pos += speed * -camera->up;
            reflectionCamera->pos -= speed * -reflectionCamera->up;
            

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

            float alfaY = sensitivity * (float)(mouseY - (height / 2)) / height;
            float alfaX = sensitivity * (float)(mouseX - (width / 2)) / width;
            glm::vec3 newOrientation = glm::rotate(camera->orientation, glm::radians(-alfaY), glm::normalize(glm::cross(camera->orientation, camera->up)));
            glm::vec3 newOrientationinv = glm::rotate(reflectionCamera->orientation, glm::radians(alfaY), glm::normalize(glm::cross(camera->orientation, camera->up)));

            if (!((glm::angle(newOrientation, camera->up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -camera->up) <= glm::radians(5.0f)))) {
                camera->orientation = newOrientation;
                reflectionCamera->orientation = newOrientationinv;
            }

            camera->orientation = glm::rotate(camera->orientation, glm::radians(-alfaX), camera->up);
            reflectionCamera->orientation = glm::rotate(reflectionCamera->orientation, glm::radians(-alfaX), reflectionCamera->up);

            
          
            
            glfwSetCursorPos(window, (width / 2), (height / 2));
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {

            firstClick = true;

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        }
    }
}

