#include "VulkanData.h"
#include "Renderer.h"
#include "VulkanFunctions.h"
#include "../ops/Logging.h"

namespace Chinstrap::ChinVulkan
{
    Kitchen::Kitchen(const VulkanContext &vulkanContext,
        const VkRenderPass& renderPass,
        const std::vector<char> &vertexShaderCode,
        const std::vector<char> &fragmentShaderCode)
            : vertexShaderCode(vertexShaderCode),
            fragmentShaderCode(fragmentShaderCode),
            vulkanContext(vulkanContext)
    {
        /* Test configuration */
        CreateKitchen(vulkanContext, *this, renderPass);
    }

    void Kitchen::Cleanup()
    {
        vkDestroyPipeline(vulkanContext.virtualGPU, pipeline, nullptr);
        vkDestroyPipelineLayout(vulkanContext.virtualGPU, pipelineLayout, nullptr);
        CHIN_LOG_INFO_VULKAN("Destroyed Kitchen and resources");
    }

    Restaurant::Restaurant(const VulkanContext &vulkanContext)
        : vulkanContext(vulkanContext)
    {
        /* Test configuration */

        auto vertShaderCode = readFile("../../../chinstrap/res/shaders/BasicVertex.spv");
        auto fragShaderCode = readFile("../../../chinstrap/res/shaders/BasicFragment.spv");
        kitchens.emplace_back(vulkanContext, renderPass, vertShaderCode, fragShaderCode);

        CreateRenderPass(vulkanContext, renderPass);
        CreateImageViews(vulkanContext, swapChainImageViews);
        CreateFramebuffers(vulkanContext, swapChainFramebuffers, swapChainImageViews, renderPass);

        CreateCommandPool(vulkanContext, commandPool);
        CreateCommandBuffer(vulkanContext, commandBuffer, commandPool);

        /* End Of Test configuration */
    }

    Restaurant::~Restaurant()
    {
        vkDestroyCommandPool(vulkanContext.virtualGPU, commandPool, nullptr);
        for (auto &framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(vulkanContext.virtualGPU, framebuffer, nullptr);
        }
        for (auto &kitchen : kitchens)
        {
            kitchen.Cleanup();
        }
        vkDestroyRenderPass(vulkanContext.virtualGPU, renderPass, nullptr);
        for (auto &imageView : swapChainImageViews)
        {
            vkDestroyImageView(vulkanContext.virtualGPU, imageView, nullptr);
        }
        CHIN_LOG_INFO_VULKAN("Destroyed Restaurant and resources");
    }
}
