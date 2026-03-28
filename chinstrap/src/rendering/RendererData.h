#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "VulkanData.h"
#include "GLFW/glfw3.h"

#include "../resourcer/ResourceRef.h"
#include "../memory/FilePathMap.h"

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

    // TODO: Let's actually not have shaders as resources, but only defined as file-paths within materials.
    //       We can instead have a global array of shaderModules paired with filepath hashID.
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
        ~Shader();
    };

    bool ShaderLoader(Shader* dataPtr, std::string_view OSFilePath);

    struct Material
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        Memory::FilePath vertexShaderPath;
        Memory::FilePath fragmentShaderPath;

        void ExampleCreateMaterial();
        bool Create();

        const ChinVulkan::VulkanContext* pVulkanContext = nullptr;
        Material(Memory::FilePath& vertexShaderPath_arg, Memory::FilePath& fragmentShaderPath_arg);

        void Cleanup();
    };

    bool MaterialLoader(Material* dataPtr, std::string_view OSFilePath);

}
