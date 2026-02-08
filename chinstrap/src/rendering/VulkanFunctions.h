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
    void CreateImageViews(const VulkanContext &vulkanContext, std::vector<VkImageView> &imageViews);

    void PickPhysicalGPU(VulkanContext &vulkanContext);
    void CreateVirtualGPU(VulkanContext &vulkanContext);

    void CreateKitchen(const VulkanContext &vulkanContext, Kitchen &kitchen);

    void CreateCommandPool(const VulkanContext &vulkanContext, VkCommandPool& commandPool);
    void CreateCommandBuffer(const VulkanContext &vulkanContext, VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool);

    void TestRecordCommandBuffer(VkCommandBuffer &targetCommandBuffer, const Restaurant& restaurant, const Kitchen& kitchen, uint32_t imageIndex);
    void CreateSyncObjects(VulkanContext &vulkanContext);

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
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
                break;
        }
        return VK_FALSE;
    }
#endif
}
