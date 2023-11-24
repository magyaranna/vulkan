
#include "app.h"
#include <fstream>


namespace v {


    App::App() {
        glfwSetWindowUserPointer(window.getWindow(), this);
        glfwSetFramebufferSizeCallback(window.getWindow(), framebufferResizeCallback);

    }

    App::~App() {

    }

    void App::run() {

        gui = std::make_unique<Gui>(window, device, *renderer.swapchain, descriptorPool.getDescriptorPool());

        /*camera*/
        Binding camera_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
        std::unique_ptr<DescriptorSetLayout> camera_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, camera_binding.bindings);
        camera = std::make_unique<Camera>(device, *renderer.swapchain, window.getWidth(),
            window.getHeight(), glm::vec3(0.0f, 10.0f, 10.0f), camera_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());

        /*texture*/
        auto texture_bindings = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);  //texture
        std::unique_ptr<DescriptorSetLayout> texture_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, texture_bindings.bindings);
       
        /*normalmap*/
        auto normalmap_bindings = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT); //normalmap
        std::unique_ptr<DescriptorSetLayout> normalmap_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, normalmap_bindings.bindings);

     // std::shared_ptr<Model> sponza_model = std::make_shared<Model>(device, "models/sponza.obj", texture_descriptorLayout->getDescriptorSetLayout(), normalmap_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::shared_ptr<Model> tree_model = std::make_shared<Model>(device, "models/tree.obj", texture_descriptorLayout->getDescriptorSetLayout(), normalmap_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        //std::shared_ptr<Model> cat_model = std::make_shared<Model>(device, "models/cat.obj", texture_descriptorLayout->getDescriptorSetLayout(), normalmap_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        //std::shared_ptr<Model> house_model = std::make_shared<Model>(device, "models/house.obj", texture_descriptorLayout->getDescriptorSetLayout(), normalmap_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        /*gameobj*/
        auto gameobject_bindings = Binding()
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);  
        std::unique_ptr<DescriptorSetLayout> gameobject_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, gameobject_bindings.bindings);
        std::unique_ptr<GameObject> obj1 = std::make_unique<GameObject>(0, device, glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, -3.1f, 0.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        //std::unique_ptr<GameObject> obj2 = std::make_unique<GameObject>(1, device, glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(20.0f, -3.0f, -10.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
      //  std::unique_ptr<GameObject> sponza_obj = std::make_unique<GameObject>(1, device, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.0f, -3.0f, 0.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
       // std::unique_ptr<GameObject> cat = std::make_unique<GameObject>(0, device, glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, -3.1f, 0.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::unique_ptr<GameObject> house = std::make_unique<GameObject>(0, device, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, -3.1f, 0.0f), gameobject_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


        obj1->model = tree_model;
       // obj2->model = tree_model;
       // cat->model = cat_model;
        //house->model = house_model;
        
       // gameobjects.emplace(0, std::move(obj1));
       // gameobjects.emplace(1, std::move(obj2));
        gameobjects.emplace(0, std::move(obj1));

        /*terrain*/
        terrain = std::make_unique<Terrain>(device, glm::vec3(0.5f, 0.5f, 0.5f), gameobject_descriptorLayout->getDescriptorSetLayout(), texture_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());

        /*light*/
        Binding light_binding = Binding()
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL);
        std::unique_ptr<DescriptorSetLayout> light_descriptorLayout = std::make_unique<DescriptorSetLayout>(device, light_binding.bindings);
        light = std::make_unique<Light>(device, light_descriptorLayout->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


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
        std::vector<VkDescriptorSet> vcsmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, varianceCascadeShadowMap,  blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());
        std::vector<VkDescriptorSet> ecsmDescriptorSets = ShadowMap::createDescriptorSets(device, 20, expCascadeShadowMap,  blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool());


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



        RenderSystem renderSystem{ device, renderer.swapchain->getRenderPass(),descriptorLayouts };
        TerrainRenderSystem terrainRenderSystem{ device,renderer.swapchain->getRenderPass(),
            {descriptorLayouts[0],  descriptorLayouts[3], descriptorLayouts[4], descriptorLayouts[5], descriptorLayouts[6],  descriptorLayouts[7], descriptorLayouts[8], descriptorLayouts[9] } };


        OffScreenRenderSystem offscreenRenderSystem{ device, {  descriptorLayouts[1],descriptorLayouts[3], descriptorLayouts[4]},  renderer.depthRenderPass };
        VSM_RenderSystem vsmRenderSystem{ device,{ descriptorLayouts[5] },  renderer.colorRenderPass };
        ESM_RenderSystem esmRenderSystem{ device,{ descriptorLayouts[5] } , renderer.colorRenderPass };
        CascadeShadowRenderSystem cascadeRenderSystem{ device, {descriptorLayouts[1], descriptorLayouts[3], descriptorLayouts[7]}, {descriptorLayouts[6]},descriptorPool.getDescriptorPool(),  renderer.depthRenderPass,  renderer.colorRenderPass };
        BlurSystem blurSystem{ device, 20, {blur_descriptorLayouts->getDescriptorSetLayout()}, blur_descriptorLayouts->getDescriptorSetLayout(), descriptorPool.getDescriptorPool(), renderer.colorRenderPass };
    
        int frameIndex = 0;
        while (!window.shouldClose()) {

            frameIndex = renderer.getFrameIndex();
            glfwPollEvents();

            bool isImGuiWindowHovered = gui->isHovered();

            if (!isImGuiWindowHovered) {
                camera->Inputs(window.getWindow());
            }

            glfwPollEvents();

            gui->updateGui(*light.get());

            if (auto commandBuffer = renderer.beginFrame()) {       //acquireNextImage + begincmd

                //update uniforms
                for (int i = 0; i < gameobjects.size(); i++) {
                    gameobjects.at(i)->updateUniformBuffer(frameIndex, gui->spin);
                }
                light->updateLightUniformBuffer(frameIndex);
                camera->updateGlobalUniformBuffer(frameIndex);
                light->updateLightVPUniformBuffer(frameIndex);
                terrain->updateUniformBuffer(frameIndex, gui->spin);

                cascadeRenderSystem.updateCascades(camera, light->getDir(), gui->splitLambda);
                cascadeRenderSystem.updateUniformBuffers(frameIndex);


                vkDeviceWaitIdle(device.getLogicalDevice());


                OffScreenRenderInfo offscreenRenderInfo{ commandBuffer,frameIndex, renderer.depthRenderPass, light,*gui, terrain, gameobjects,};
                {
                    offscreenRenderSystem.renderGameObjects(offscreenRenderInfo, simpleDepthShadowMap);
                    cascadeRenderSystem.renderGameObjects(offscreenRenderInfo, cascadeShadowMap);
                }

                offscreenRenderInfo.renderPass = renderer.colorRenderPass;
                {
                    vsmRenderSystem.renderGameObjects(offscreenRenderInfo, varianceShadowMap, simpleDepthShadowMap.getDescriptorSet(frameIndex));
                    esmRenderSystem.renderGameObjects(offscreenRenderInfo, expShadowMap, simpleDepthShadowMap.getDescriptorSet(frameIndex));

                    if (gui->cascade && gui->vsm) cascadeRenderSystem.renderGameObjects(offscreenRenderInfo, varianceCascadeShadowMap, cascadeShadowMap);
                    if (gui->cascade && gui->esm) cascadeRenderSystem.renderGameObjects(offscreenRenderInfo, expCascadeShadowMap, cascadeShadowMap);

                    if (gui->blur) {
                        blurSystem.render(commandBuffer, frameIndex, varianceShadowMap, vsmDescriptorSets[frameIndex], renderer.colorRenderPass);
                        blurSystem.render(commandBuffer, frameIndex, expShadowMap, esmDescriptorSets[frameIndex], renderer.colorRenderPass);
                        blurSystem.render(commandBuffer, frameIndex, varianceCascadeShadowMap, vcsmDescriptorSets[frameIndex], renderer.colorRenderPass);
                        blurSystem.render(commandBuffer, frameIndex, expCascadeShadowMap, ecsmDescriptorSets[frameIndex], renderer.colorRenderPass);
                       
                    }
                  VkImageMemoryBarrier imageBarrier = {};
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;  
                    imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
                    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.image = varianceCascadeShadowMap.image;
                    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
                    imageBarrier.subresourceRange.baseMipLevel = 0;
                    imageBarrier.subresourceRange.levelCount = 1;
                    imageBarrier.subresourceRange.baseArrayLayer = 0;
                    imageBarrier.subresourceRange.layerCount = 4;

                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

                    imageBarrier.image = expCascadeShadowMap.image;
                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
                    
                }

                renderer.beginSwapChainRenderPass(commandBuffer);
                {
                    VkDescriptorSet csm = cascadeShadowMap.getDescriptorSet(frameIndex);
                    if (gui->cascade && gui->vsm)
                        csm = varianceCascadeShadowMap.getDescriptorSet(frameIndex);
                    else if (gui->cascade && gui->esm)
                        csm = expCascadeShadowMap.getDescriptorSet(frameIndex);

                    RenderInfo renderInfo{
                        camera, light, *gui, terrain, gameobjects,
                        simpleDepthShadowMap.getDescriptorSet(frameIndex),
                        csm,
                        cascadeRenderSystem.getMXDescriptorSet(frameIndex),
                        varianceShadowMap.getDescriptorSet(frameIndex),
                        expShadowMap.getDescriptorSet(frameIndex)
                    };

                    terrainRenderSystem.renderTerrain(commandBuffer, frameIndex, renderInfo);

                    renderSystem.renderGameObjects(commandBuffer, frameIndex, renderInfo);
                }
                renderer.endRenderPass(commandBuffer);

                

                renderer.endFrame();     //submit

               
                if (gui->getQueryResults) {
                    cascadeRenderSystem.ts->getQueryResults();

                }
                else if(gui->stopQuery && !writeFile) {
                    std::ofstream myfile("timestamps.xls");
                    int vsize = cascadeRenderSystem.ts->deltatimes.size();
                    for (int n = 0 ; n < vsize; n++)
                        myfile << cascadeRenderSystem.ts->deltatimes[n] << '\r';

                    writeFile = true;
                }
                
            }
        }
        vkDeviceWaitIdle(device.getLogicalDevice());
    }


    void App::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->renderer.swapchain->setFramebufferResized(true);

    }
}

