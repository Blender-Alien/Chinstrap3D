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

void Chinstrap::ChinVulkan::Restaurant::Initialize(const VulkanContext* inputVulkanContext)
{
    this->pVulkanContext = inputVulkanContext;

    /* Test configuration */
    auto vertShaderCode = readFile("../../../chinstrap/res/shaders/BasicVertex.spv");
    auto fragShaderCode = readFile("../../../chinstrap/res/shaders/BasicFragment.spv");
    materials.emplace_back(*pVulkanContext, vertShaderCode, fragShaderCode);

    commandPool = (ExampleCreateCommandPool(*pVulkanContext));
    ExampleCreateCommandBuffers(*pVulkanContext, commandBuffers, commandPool);
    /* End Of Test configuration */
}

void Chinstrap::ChinVulkan::Restaurant::Cleanup()
{
    vkDestroyCommandPool(pVulkanContext->virtualGPU, commandPool, nullptr);
    for (auto &material: materials)
    {
        material.Cleanup();
    }
    CHIN_LOG_INFO_VULKAN("Destroyed Restaurant and resources");
}