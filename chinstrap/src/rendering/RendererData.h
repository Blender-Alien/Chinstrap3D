#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "GLFW/glfw3.h"

#include "../resourcer/ResourceManager.h"

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

    struct Shader
    {
        enum class ShaderType
        {
            VERTEX, FRAGMENT
        };

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        ShaderType shaderType;

        bool Create(const ChinVulkan::VulkanContext &vulkanContext, const char* codeBegin, const char* codeEnd);

        explicit Shader(const ShaderType shaderType)
            : shaderType(shaderType) {}
    };

    struct Material
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        Resourcer::ResourceRef vertexShaderRef;
        Resourcer::ResourceRef fragmentShaderRef;

        const ChinVulkan::VulkanContext &vulkanContext;
        Material(const ChinVulkan::VulkanContext &vulkanContext,
            const Resourcer::ResourceRef& vertexShaderRef_arg, const Resourcer::ResourceRef& fragmentShaderRef_arg);

        void Cleanup();
    };
    void ExampleCreateMaterial(const ChinVulkan::VulkanContext &vulkanContext, Material &material, const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode);

}
