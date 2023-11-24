#pragma once

#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "light.h"

namespace v {

    class Gui {

    public:

        //bool displayShadowmap = false;
        bool pcf = false;
        bool bias = true;
        bool displayNormalmap = false;
        bool spin = false;
        //bool acneRemoved = true;
        //bool sponza = false;
        bool cascade = false;
        bool cascadecolor = false;
        float splitLambda = 0.2f;

        bool vsm = false;
        bool esm = false;

        bool frontface = false;
        bool blur = false;

        bool getQueryResults = false;
        bool stopQuery = false;


        Gui(Window& window, Device& device, SwapChain& swapchain, VkDescriptorPool pool);
        ~Gui();

        bool isHovered();
        void updateGui(Light& light);
        void renderGui(VkCommandBuffer cmd);


    };



}