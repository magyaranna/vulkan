#pragma once

#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "light.h"

namespace v {

    class Gui {

    public:

        bool pcf = false;
        bool bias = true;
        bool displayNormalmap = false;
        bool spin = false;

        float dFactor = 10.0f;
        bool wireframe = true;

        bool cascade = false;
        bool cascadecolor = false;
        float splitLambda = 0.2f;

        bool vsm = false;
        bool esm = false;

        bool frontface = false;
        bool blur = false;

        bool getQueryResults = false;
        bool stopQuery = false;

        int clicked = 0;

        Gui(Window& window, Device& device, SwapChain& swapchain, VkDescriptorPool pool);
        ~Gui();

        bool isHovered();
        void updateGui(Light& light);
        void renderGui(VkCommandBuffer cmd);


    };



}