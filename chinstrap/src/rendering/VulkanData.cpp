#include "VulkanData.h"
#include "Renderer.h"
#include "VulkanFunctions.h"
#include "../ops/Logging.h"

namespace Chinstrap::ChinVulkan
{
    Material::Material(const VulkanContext &vulkanContext,
        const std::vector<char> &vertexShaderCode,
        const std::vector<char> &fragmentShaderCode)
            : vertexShaderCode(vertexShaderCode),
            fragmentShaderCode(fragmentShaderCode),
            vulkanContext(vulkanContext)
    {
        /* Test configuration */
        ExampleCreateMaterial(vulkanContext, *this);
    }

    void Material::Cleanup()
    {
        vkDestroyPipeline(vulkanContext.virtualGPU, pipeline, nullptr);
        vkDestroyPipelineLayout(vulkanContext.virtualGPU, pipelineLayout, nullptr);
        CHIN_LOG_INFO_VULKAN("Destroyed Material and resources");
    }

    Restaurant::Restaurant(const VulkanContext &vulkanContext)
        : vulkanContext(vulkanContext)
    {
        /* Test configuration */

        auto vertShaderCode = readFile("../../../chinstrap/res/shaders/BasicVertex.spv");
        auto fragShaderCode = readFile("../../../chinstrap/res/shaders/BasicFragment.spv");
        materials.emplace_back(vulkanContext, vertShaderCode, fragShaderCode);

        ExampleCreateImageViews(vulkanContext, swapChainImageViews);

        ExampleCreateCommandPool(vulkanContext, commandPool);
        ExampleCreateCommandBuffer(vulkanContext, commandBuffer, commandPool);

        /* End Of Test configuration */
    }

    Restaurant::~Restaurant()
    {
        vkDestroyCommandPool(vulkanContext.virtualGPU, commandPool, nullptr);
        for (auto &material : materials)
        {
            material.Cleanup();
        }
        for (auto &imageView : swapChainImageViews)
        {
            vkDestroyImageView(vulkanContext.virtualGPU, imageView, nullptr);
        }
        CHIN_LOG_INFO_VULKAN("Destroyed Restaurant and resources");
    }
}
