#pragma once

#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Chinstrap::ChinVulkan
{
    struct VulkanContext
    {
        VkInstance instance;
        VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
        VkDevice virtualGPU = VK_NULL_HANDLE;
        VkSurfaceKHR renderSurface = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkImageView> swapChainImageViews;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;

        [[nodiscard]] bool isComplete() const
        {
            return graphicsFamily.has_value() && presentationFamily.has_value();
        }
    };
}