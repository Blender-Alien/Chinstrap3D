#pragma once

#define GLFW_INCLUDE_VULKAN
#include "../ops/Logging.h"
#include "GLFW/glfw3.h"

namespace Chinstrap::Window {struct Frame;}
namespace Chinstrap::ChinVulkan {struct QueueFamilyIndices; struct VulkanSetupData;}

namespace Chinstrap::ChinVulkan
{
    void Init(VulkanSetupData &vulkanData, const std::string& name);
    void Shutdown(VulkanSetupData &vulkanData);

    void PickPhysicalGPU(VulkanSetupData &data);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void CreateVirtualGPU(VulkanSetupData &vulkanData);


    //TODO: 'vkCreateInstance' & 'vkDestroyInstance' Debug functionality
#ifdef CHIN_VK_VAL_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        CHIN_LOG_ERROR("Vulkan validation layer: >>>{}<<<", pCallbackData->pMessage);
        return VK_FALSE;
    }
#endif
}
