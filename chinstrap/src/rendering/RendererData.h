#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "GLFW/glfw3.h"

#include <array>

namespace Chinstrap::Renderer
{
    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;
    };
    VkVertexInputBindingDescription GetVertexBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescriptions();

    struct Material
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        const ChinVulkan::VulkanContext &vulkanContext;
        Material(const ChinVulkan::VulkanContext &vulkanContext,
            const std::vector<char>& vertexShaderCode,
            const std::vector<char>& fragmentShaderCode);

        void Cleanup();
    };
    void ExampleCreateMaterial(const ChinVulkan::VulkanContext &vulkanContext, Material &material, const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode);

}
