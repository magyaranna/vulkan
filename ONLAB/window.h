#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>



namespace v {

    class Window {
    private:
        
        GLFWwindow* window;
        
        const int width;
        const int height;
        std::string name;


    public:
        Window(int w, int h, std::string name);
        ~Window();

        Window(const Window&) = delete;
        void operator=(const Window&) = delete;

        bool shouldClose(){ return glfwWindowShouldClose(window); }
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

        GLFWwindow* getWindow() { return window; }
        int getWidth() { return width; }
        int getHeight() { return height; }
        
    };


}