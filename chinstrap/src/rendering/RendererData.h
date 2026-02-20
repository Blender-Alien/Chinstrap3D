#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "GLFW/glfw3.h"

namespace Chinstrap::Renderer
{
    /*
    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription GetBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };
    */

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
