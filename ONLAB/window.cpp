#include "window.h"
#include <stdexcept>


namespace v {

    Window::Window(int w, int h, std::string name) : width{ w }, height{ h }, name{ name } {
        
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    }

    Window::~Window() {

        glfwDestroyWindow(window);
        glfwTerminate();

    }
    

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
         if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }

    }

   

}