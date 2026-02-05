#pragma once

#define GLFW_INCLUDE_VULKAN
#include "../ops/Logging.h"
#include "GLFW/glfw3.h"

namespace Chinstrap::Window {struct Frame;}

namespace Chinstrap::Vulkan
{
    void Init(Window::Frame& frame);
    void Shutdown(Window::Frame& frame);

    //TODO: 'vkCreateInstance' & 'vkDestroyInstance' Debug functionality

#ifdef CHIN_VK_VAL_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        CHIN_LOG_ERROR("Vulkan validation layer: %s", pCallbackData->pMessage);
        return VK_FALSE;
    }
#endif
}
