#pragma once

#include "spdlog/fmt/bundled/base.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "VulkanData.h"
#include "../ops/Logging.h"

namespace Chinstrap::Window {struct Frame;}
namespace Chinstrap::ChinVulkan {struct VulkanContext;}

/* Functions to call from some an App or Frame context to startup and shutdown Vulkan*/
namespace Chinstrap::ChinVulkan
{
    // Completely initialize a frames vulkanContext 
    bool Initialize(Window::Frame &frame);

    // Deallocate everything inside the vulkanContext object, as well as globally enabled validation layers
    // Other allocated Vulkan objects must be properly deallocated in by their respective containers
    void Shutdown(VulkanContext &vulkanContext);
}

/* Vulkan helper functions */
namespace Chinstrap::ChinVulkan
{
    // Create VulkanContext make sure the API minimum requirements are met, enable validation layers
    bool CreateContext(VulkanContext &vulkanContext, const std::string& name);

    bool CreateSurface(Window::Frame &frame);

    bool CreateSwapChain(Window::Frame &frame);

    bool CreateDefaultImageViews(VulkanContext &vulkanContext);

    // For when the window surface changes due to user input
    bool RecreateSwapChain(Window::Frame &frame);

    void CleanupSwapChain(VulkanContext &vulkanContext);

    // Function for automatically picking a GPU at startup, with a few (arbitrary) requirements,
    // the user may choose one later through a different system
    bool AutoPickPhysicalGPU(VulkanContext &vulkanContext);

    bool CreateVirtualGPU(VulkanContext &vulkanContext);

    bool CreateSyncObjects(VulkanContext &vulkanContext);
}

/* Some example functions that facilitate creating a material */
namespace Chinstrap::ChinVulkan
{
    void ExampleCreateMaterial(const VulkanContext &vulkanContext, Material &material, const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode);

    VkCommandPool ExampleCreateCommandPool(const VulkanContext &vulkanContext);

    void ExampleCreateCommandBuffers(const VulkanContext &vulkanContext, std::vector<VkCommandBuffer>& buffers, const VkCommandPool &commandPool);

    void ExampleRecordCommandBuffer(VkCommandBuffer &targetCommandBuffer, uint32_t imageIndex, const Restaurant& restaurant, const Material& material, const VulkanContext &vulkanContext);

}

/* ImGUI related rendering functions */
namespace Chinstrap::ChinVulkan
{
    void RecordImGUICommandBuffer(VkCommandBuffer& targetCommandBuffer, const VkImageView &targetImageView,
                                  const ChinVulkan::Restaurant &restaurant, const ChinVulkan::Material &material);
}
/* Global Vulkan helper functions */
namespace Chinstrap::ChinVulkan
{
    // TODO: Handle shaders properly
    inline VkShaderModule CreateShaderModule(const VulkanContext &vulkanContext, const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vulkanContext.virtualGPU, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            CHIN_LOG_ERROR("Failed to create shader module!");
        }
        return shaderModule;
    }
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
