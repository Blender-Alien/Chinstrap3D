#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Chinstrap::ChinVulkan
{
    struct VulkanContext
    {
        VkInstance instance;
        uint32_t instanceSupportedVersion;

        VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
        VkSurfaceKHR windowSurface = VK_NULL_HANDLE;

        VkDevice virtualGPU = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        uint32_t currentFrame = 0;
        std::vector<VkImageView> defaultImageViews;
        bool frameResized = false;

        // We need vectors of semaphores/fences to support > 1 frames in flight
        const int MAX_FRAMES_IN_FLIGHT = 2;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> firstStageFinishedSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        std::vector<const char*> neededDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VkDescriptorPool imguiPool;

        PFN_vkCmdBeginRenderingKHR PFN_vkCmdBeginRenderingKHR;
        PFN_vkCmdEndRenderingKHR PFN_vkCmdEndRenderingKHR;

        VkDebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
    };

    struct Material
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        const VulkanContext& vulkanContext;
        Material(const VulkanContext& vulkanContext,
            const std::vector<char>& vertexShaderCode,
            const std::vector<char>& fragmentShaderCode);

        void Cleanup();
    };

    struct Restaurant
    {
        std::vector<Material> materials;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

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

        [[nodiscard]] bool allSupported() const
        {
            return graphicsFamily.has_value() && presentationFamily.has_value();
        }
    };
}
