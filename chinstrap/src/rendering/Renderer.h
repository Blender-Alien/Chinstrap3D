#pragma once
#include "../ops/Logging.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../memory/StackAllocator.h"
#include "../memory/StackArray.h"
#include "VulkanData.h"
#include "RendererData.h"

namespace Chinstrap {struct Scene;}
namespace Chinstrap::Display { struct Window;}
namespace Chinstrap::UserSettings { struct GraphicsSettings;}

namespace Chinstrap::Renderer
{
    // Note: This struct is so big in size because StackArray stores all relevant stuff on the stack,
    // and this all accumulates. If it is required that structs like this be much smaller, we could
    // consider rewriting our Memory datastructures to store some of our extra context data on the heap
    // directly next to the actual data.

    struct RenderContext
    {
        // TODO: This should not be static if we intend on being able to have more than one RenderContext
        static uint32_t imageIndex;
        static uint32_t GetImageIndex() {return imageIndex;}

        Memory::StackAllocator stackAllocator;

        Memory::StackArray<ChinVulkan::SubmitData> aSubmitDatas;
        Memory::StackArray<VkSubmitInfo> aSubmitInfos;
        Memory::StackArray<VkCommandPool> aCommandPools;
        Memory::StackArray<VkFence> aFences;
        Memory::StackArray<VkSemaphore> aImageAvailableSemaphores;
        Memory::StackArray2D<VkSemaphore> aaLayerSemaphores;

        Memory::StackAllocator cmdBufferAllocator;
        Memory::StackArray2D<VkCommandBuffer> aaCmdBuffers;

        ChinVulkan::VulkanContext* pVulkanContext = nullptr;
        Display::Window* pWindow = nullptr;
        UserSettings::GraphicsSettings* pGraphicsSettings = nullptr;
        const std::vector<std::unique_ptr<Scene>>* pSceneStack = nullptr;

        bool Create(ChinVulkan::VulkanContext* vulkanContext_arg, Display::Window* window_arg,
            UserSettings::GraphicsSettings* graphicsSettings_arg, const std::vector<std::unique_ptr<Scene>>* sceneStack_arg);
        void Destroy();

        inline static bool created = false;
        explicit RenderContext()
            : aSubmitDatas(stackAllocator), aSubmitInfos(stackAllocator), aCommandPools(stackAllocator),
              aFences(stackAllocator), aImageAvailableSemaphores(stackAllocator), aaLayerSemaphores(stackAllocator),
              aaCmdBuffers(cmdBufferAllocator)
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

    // When a new scene has been created, give out pointer(s) to the buffers, so that the scene can render
    void SetupSceneCmdBuffers(uint8_t sceneIndex, RenderContext &renderContext);
}
