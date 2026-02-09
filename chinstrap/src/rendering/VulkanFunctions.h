#pragma once

#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "../ops/Logging.h"
#include "GLFW/glfw3.h"

namespace Chinstrap::Window {struct Frame;}
namespace Chinstrap::ChinVulkan {struct VulkanContext;}

/* Functions to call from some sort of App context when starting up and shutting down in the presented order*/
namespace Chinstrap::ChinVulkan
{
    // Create VulkanContext make sure the API minimum requirements are met, enable validation layers
    bool Init(VulkanContext &vulkanContext, const std::string& name);

    bool CreateSurface(Window::Frame &frame);

    bool CreateSwapChain(Window::Frame &frame);

    // Function for automatically picking a GPU at startup, with a few (arbitrary) requirements,
    // the user may choose one later through a different system
    bool AutoPickPhysicalGPU(VulkanContext &vulkanContext);

    bool CreateVirtualGPU(VulkanContext &vulkanContext);

    bool CreateSyncObjects(VulkanContext &vulkanContext);

    // Deallocate everything inside the vulkanContext object, as well as globally enabled validation layers
    // Other allocated Vulkan objects must be properly deallocated in by their respective containers
    void Shutdown(VulkanContext &vulkanContext);
}

/* Some Boilerplate code to facilitate building up */
namespace Chinstrap::ChinVulkan
{
    void ExampleCreateMaterial(const VulkanContext &vulkanContext, Material &material);

    void ExampleCreateImageViews(const VulkanContext &vulkanContext, std::vector<VkImageView> &imageViews);

    VkCommandPool ExampleCreateCommandPool(const VulkanContext &vulkanContext);

    VkCommandBuffer ExampleCreateCommandBuffer(const VulkanContext &vulkanContext, const VkCommandPool& commandPool);

    void ExampleRecordCommandBuffer(VkCommandBuffer &targetCommandBuffer, uint32_t imageIndex, const Restaurant& restaurant, const Material& material, const VulkanContext &vulkanContext);

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
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
                break;
        }
        return VK_FALSE;
    }
#endif
}
