#pragma once

#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Chinstrap::ChinVulkan
{
    struct VulkanSetupData
    {
        VkInstance instance;
        VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
        VkDevice virtualGPU = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;

        [[nodiscard]] bool isComplete() const
        {
            return graphicsFamily.has_value();
        }
    };
}