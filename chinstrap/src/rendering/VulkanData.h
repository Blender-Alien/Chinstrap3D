#pragma once

#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Chinstrap::ChinVulkan
{
    struct VulkanContext
    {
        VkInstance instance;

        VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
        VkSurfaceKHR windowSurface = VK_NULL_HANDLE;

        VkDevice virtualGPU = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
        VkFence inFlightFence = VK_NULL_HANDLE;

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        VkDebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };

    struct Kitchen
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        const std::vector<char> vertexShaderCode;
        const std::vector<char> fragmentShaderCode;

        const VulkanContext& vulkanContext;
        Kitchen(const VulkanContext& vulkanContext,
            const std::vector<char>& vertexShaderCode,
            const std::vector<char>& fragmentShaderCode);

        void Cleanup();
    };

    struct Restaurant
    {
        std::vector<Kitchen> kitchens;

        std::vector<VkImageView> swapChainImageViews;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        const VulkanContext& vulkanContext;
        explicit Restaurant(const VulkanContext& vulkanContext);
        ~Restaurant();
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