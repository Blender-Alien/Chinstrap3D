#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanData.h"
#include "../ops/Logging.h"

namespace Chinstrap::ChinVulkan {struct VulkanContext;}
namespace Chinstrap::UserSettings {struct GraphicsSettings;}

namespace Chinstrap::ChinVulkan
{ /* Functions to call from some an App or Frame context to startup and shutdown Vulkan */

    // Completely initialize a frames vulkanContext
    bool Initialize(VulkanContext& vulkanContext, GLFWwindow* glfwWindow, UserSettings::GraphicsSettings &graphicsSettings, std::string_view title);

    // Deallocate everything inside the vulkanContext object, as well as globally enabled validation layers
    // Other allocated Vulkan objects must be properly deallocated in by their respective containers
    void Shutdown(const VulkanContext &vulkanContext);
}

namespace Chinstrap::ChinVulkan
{ /* Vulkan Helper functions */

    // Create VulkanContext make sure the API minimum requirements are met, enable validation layers
    bool CreateContext(VulkanContext &vulkanContext, std::string_view name);

    bool CreateSurface(VulkanContext& vulkanContext, GLFWwindow* glfwWindow);

    bool CreateSwapChain(VulkanContext& vulkanContext, GLFWwindow* glfwWindow, UserSettings::GraphicsSettings& graphicsSettings);

    bool CreateDefaultImageViews(VulkanContext &vulkanContext);

    // For when the window surface changes due to user input
    bool RecreateSwapChain(VulkanContext& vulkanContext, GLFWwindow* glfwWindow, UserSettings::GraphicsSettings& graphicsSettings);

    void CleanupSwapChain(const VulkanContext &vulkanContext);

    // Function for automatically picking a GPU at startup, with a few (arbitrary) requirements,
    // the user may choose one later through a different system
    bool AutoPickPhysicalGPU(VulkanContext &vulkanContext);

    bool CreateVirtualGPU(VulkanContext &vulkanContext);

    bool CreateVMA(VulkanContext &vulkanContext);
}

namespace Chinstrap::ChinVulkan
{
    void CreateCommandPool(const VulkanContext &vulkanContext, VkCommandPool* commandPool);
    void CreateCommandBuffer(const VulkanContext &vulkanContext, VkCommandBuffer* buffer, const VkCommandPool* commandPool);

    void BeginRendering(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext, const VkPipeline& pipeline);
    void ExampleRecordCommandBuffer(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext, const VkPipeline& pipeline);
    void EndRendering(const VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext, VkImageLayout newLayout);
}

namespace Chinstrap::ChinVulkan
{ /* DevInterface related rendering functions ( ImGui ) */
    void ExampleRecordDevInterfaceCommandBuffer(const VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext);
}

namespace Chinstrap::ChinVulkan
{ /* Vulkan validation layers debugCallback function */
    //TODO: 'vkCreateInstance' & 'vkDestroyInstance' Debug functionality
#ifdef CHIN_VK_VAL_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                CHIN_LOG_ERROR("[Vulkan validation layer] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                CHIN_LOG_WARN("[Vulkan validation layer] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                CHIN_LOG_INFO("[Vulkan validation layer] {}", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                CHIN_LOG_INFO("[Vulkan validation layer] {}", pCallbackData->pMessage);
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
                CHIN_LOG_INFO("[Vulkan validation layer] {}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }
#endif
}
