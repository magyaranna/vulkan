#pragma once

#include "vulkan/vulkan.h"

#include "terrain.h"
#include "camera.h"
#include "light.h"
#include "gui.h"
#include "gameobject.h"

namespace v {

    struct RenderInfo {
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
    };
}