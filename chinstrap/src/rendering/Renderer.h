#pragma once
#include "../ops/Logging.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../memory/StackAllocator.h"
#include "../memory/Array.h"

#include <fstream>

#include "VulkanData.h"

namespace Chinstrap {struct Scene;}

namespace Chinstrap::Renderer
{
    struct RenderContext
    {
        std::vector<VkSubmitInfo> tempSubmits;

        static uint32_t imageIndex;
        static uint32_t GetImageIndex() {return imageIndex;}

        Memory::StackAllocator stackAllocator;

        Memory::StackArray<sizeof(ChinVulkan::SubmitData)> aSubmitDatas;
        Memory::StackArray<sizeof(VkSubmitInfo)> aSubmitInfos;
        Memory::StackArray<sizeof(VkCommandPool)> aCommandPools;
        Memory::StackArray<sizeof(VkFence)> aFences;
        Memory::StackArray<sizeof(VkSemaphore)> aImageAvailableSemaphores;
        Memory::StackArray2D<sizeof(VkSemaphore)> aaLayerSemaphores;

        Memory::StackAllocator cmdBufferAllocator;
        Memory::StackArray2D<sizeof(VkCommandBuffer)> aCmdBuffers;

        ChinVulkan::VulkanContext* pVulkanContext = nullptr;
        const std::vector<std::unique_ptr<Scene>>* pSceneStack = nullptr;

        void Create(uint8_t sceneStackSize);
        void Destroy();

        inline static bool created = false;
        explicit RenderContext()
            : aSubmitDatas(stackAllocator), aSubmitInfos(stackAllocator), aCommandPools(stackAllocator),
              aFences(stackAllocator), aImageAvailableSemaphores(stackAllocator), aaLayerSemaphores(stackAllocator),
              aCmdBuffers(cmdBufferAllocator)
        {
            assert(!created); // We only support one render context
            created = true;
        }
        void operator=(const RenderContext&) = delete;
        RenderContext(RenderContext& other) = delete;
    };
}

namespace Chinstrap::Renderer
{
    // Return true, if currentFrame needs to be skipped
    [[nodiscard]] bool BeginFrame(uint32_t currentFrame, RenderContext &renderContext);

    void SubmitDrawData(uint32_t currentFrame, RenderContext &renderContext);

    void RenderFrame(uint32_t currentFrame, RenderContext &renderContext);
}

// Temporary
static std::vector<char> readFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        CHIN_LOG_ERROR("Failed to open file {}!", filePath);
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
