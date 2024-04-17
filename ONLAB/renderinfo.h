#pragma once

#include "vulkan/vulkan.h"

#include "terrain.h"
#include "camera.h"
#include "light.h"
#include "gui.h"
#include "gameobject.h"
#include "water.h"

namespace v {

    struct RenderInfo {
        glm::vec2 viweport;
        std::unique_ptr<Camera> const& camera;
        std::unique_ptr<Light> const& light;
        Gui& gui;
        std::unique_ptr<Terrain> const& terrain;
        std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects;

        VkDescriptorSet simpleShadowMap;
        VkDescriptorSet cascadeShadowmap;
        VkDescriptorSet cascadeLightSpaceMx;
        VkDescriptorSet vsmShadowmap;
        VkDescriptorSet esmShadowmap;

        std::unique_ptr<Water> const& water;
    };

    struct OffScreenRenderInfo {
        VkCommandBuffer& cmd;
        int currentFrame;
        VkRenderPass renderPass;
        std::unique_ptr<Light> const& light;
        Gui& gui;
        std::unique_ptr<Terrain> const& terrain;
        std::unordered_map<unsigned int, std::unique_ptr<GameObject>>& gameobjects;
    };
}