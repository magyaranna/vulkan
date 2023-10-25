
#include "gui.h"
#include <imconfig.h>
#include <imgui_tables.cpp>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <imgui_demo.cpp>
#include <imgui_impl_glfw.cpp>

#include <imgui_impl_vulkan_but_better.h>


namespace v {

    Gui::Gui(Window& window, Device& device, SwapChain& swapchain, VkDescriptorPool pool) {

        ImGui::CreateContext();

        ImGuiIO* IO = &ImGui::GetIO();
        IO->WantCaptureMouse;
        IO->WantCaptureKeyboard;
        // IO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

        ImGui_ImplVulkan_InitInfo info;
        info.DescriptorPool = pool;
        info.RenderPass = swapchain.getRenderPass();
        info.Device = device.getLogicalDevice();
        info.PhysicalDevice = device.getPhysicalDevice();
        info.ImageCount = swapchain.MAX_FRAMES_IN_FLIGHT;
        info.MsaaSamples = device.getMSAASampleCountFlag();
        ImGui_ImplVulkan_Init(&info);

        VkCommandBuffer commandBuffer = Helper::beginSingleTimeCommands(device);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        Helper::endSingleTimeCommands(device, commandBuffer);

        vkDeviceWaitIdle(device.getLogicalDevice());
        ImGui_ImplVulkan_DestroyFontUploadObjects();

    }
    Gui::~Gui() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

    }


    void Gui::updateGui(Light& light) {

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        ImGui::Checkbox("display normalmap", &displayNormalmap);
        ImGui::Checkbox("spin", &spin);

        ImGui::Checkbox("cascade", &cascade);
        ImGui::Checkbox("csm color", &cascadecolor);
        ImGui::Checkbox("vsm", &vsm);
        ImGui::Checkbox("esm", &esm);

        // Sliders
        static ImGuiSliderFlags flags = ImGuiSliderFlags_None;

        static float slider_f = 0.5f;
        static int slider_i = 50;



        glm::vec3 dir = light.getDir();
        static float x = dir.x, y = dir.y, z = dir.z;

        ImGui::Text("");

        ImGui::Text("Light direction: %f %f", x, z);


        ImGui::SliderFloat("x (-1 -> 1)", &x, -1.0f, 1.0f, "%.3f", flags);
        // ImGui::SliderFloat("y (-1 -> 1)", &y, -1.0f, 1.0f, "%.3f", flags);
        ImGui::SliderFloat("z (-1 -> 1)", &z, -1.0f, 1.0f, "%.3f", flags);

        light.setDirection(glm::vec3(x, -1, z));


        //ImGui::ShowDemoWindow();

        ImGui::Render();

    }


    void Gui::renderGui(VkCommandBuffer cmd) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd, 0, NULL);
    }
    bool Gui::isHovered() {

        return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    }
}