#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "GLFW/glfw3.h"

#include "../memory/StringMap.h"

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

    bool CreateShader(const ChinVulkan::VulkanContext &vulkanContext,VkShaderModule& shaderModule, const char* codeBegin, const char* codeEnd);
    bool ShaderLoader(VkShaderModule& shaderModule, std::string_view OSFilePath);

    struct Material
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        Memory::DevString vertexShaderPath; // @Serialized
        Memory::DevString fragmentShaderPath; // @Serialized

        void ExampleCreateMaterial(VkShaderModule vertexShader, VkShaderModule fragmentShader);

        bool Create(ChinVulkan::VulkanContext* vulkanContext);

        const ChinVulkan::VulkanContext* pVulkanContext = nullptr;
        Material(const Memory::DevString& vertexShaderPath_arg, const Memory::DevString& fragmentShaderPath_arg);
        ~Material();
    };

    bool MaterialLoader(Material* dataPtr, std::string_view OSFilePath);
}
