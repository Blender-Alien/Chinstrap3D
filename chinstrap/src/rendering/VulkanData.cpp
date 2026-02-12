#include "VulkanData.h"
#include "Renderer.h"
#include "VulkanFunctions.h"
#include "../ops/Logging.h"

Chinstrap::ChinVulkan::Material::Material(const VulkanContext &vulkanContext,
                   const std::vector<char> &vertexShaderCode,
                   const std::vector<char> &fragmentShaderCode)
    : vulkanContext(vulkanContext)
{
    /* Test configuration */
    ExampleCreateMaterial(vulkanContext, *this, vertexShaderCode, fragmentShaderCode);
}

void Chinstrap::ChinVulkan::Material::Cleanup()
{
    vkDestroyPipeline(vulkanContext.virtualGPU, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkanContext.virtualGPU, pipelineLayout, nullptr);
    CHIN_LOG_INFO_VULKAN("Destroyed Material and resources");
}

Chinstrap::ChinVulkan::Restaurant::Restaurant(const VulkanContext &vulkanContext)
    : vulkanContext(vulkanContext)
{
    /* Test configuration */

    auto vertShaderCode = readFile("../../../chinstrap/res/shaders/BasicVertex.spv");
    auto fragShaderCode = readFile("../../../chinstrap/res/shaders/BasicFragment.spv");
    materials.emplace_back(vulkanContext, vertShaderCode, fragShaderCode);

    ExampleCreateImageViews(vulkanContext, swapChainImageViews);

    commandPool = (ExampleCreateCommandPool(vulkanContext));
    commandBuffers.push_back(ExampleCreateCommandBuffer(vulkanContext, commandPool));

    /* End Of Test configuration */
}

Chinstrap::ChinVulkan::Restaurant::~Restaurant()
{
    vkDestroyCommandPool(vulkanContext.virtualGPU, commandPool, nullptr);
    for (auto &material: materials)
    {
        material.Cleanup();
    }
    for (auto &imageView: swapChainImageViews)
    {
        vkDestroyImageView(vulkanContext.virtualGPU, imageView, nullptr);
    }
    CHIN_LOG_INFO_VULKAN("Destroyed Restaurant and resources");
}
