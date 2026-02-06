#pragma once

#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "../ops/Logging.h"
#include "GLFW/glfw3.h"

namespace Chinstrap::Window {struct Frame;}
namespace Chinstrap::ChinVulkan {struct QueueFamilyIndices; struct VulkanContext;}

namespace Chinstrap::ChinVulkan
{
    void Init(VulkanContext &vulkanContext, const std::string& name);
    void Shutdown(VulkanContext &vulkanContext);

    void CreateSurface(Window::Frame &frame);
    void CreateSwapChain(Window::Frame &frame);

    void PickPhysicalGPU(VulkanContext &vulkanContext);
    void CreateVirtualGPU(VulkanContext &vulkanContext);

    //TODO: 'vkCreateInstance' & 'vkDestroyInstance' Debug functionality
#ifdef CHIN_VK_VAL_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        CHIN_LOG_ERROR("[Vulkan validation layer] {}", pCallbackData->pMessage);
        return VK_FALSE;
    }
#endif
}
