#include "Renderer.h"

#include "VulkanFunctions.h"
#include "../Application.h"
#include "../Scene.h"

uint32_t Chinstrap::Renderer::RenderContext::imageIndex = 0;

void Chinstrap::Renderer::RenderContext::Create(const uint8_t sceneStackSize)
{
    pVulkanContext = &Application::App::Get().frame.vulkanContext;
    pSceneStack = &Application::App::Get().GetSceneStack();

    {
        uint32_t allocatorSize = 0;
        { // We need all of these per scene
            allocatorSize += sizeof( ChinVulkan::SubmitData[sceneStackSize] );
            allocatorSize += sizeof( VkSubmitInfo[sceneStackSize] );
            allocatorSize += sizeof( VkCommandPool[sceneStackSize] );
            allocatorSize += sizeof( VkSemaphore[pVulkanContext->swapChainImages.size()][sceneStackSize] );
        }
        allocatorSize += sizeof( VkFence[pVulkanContext->MAX_FRAMES_IN_FLIGHT] );
        allocatorSize += sizeof( VkSemaphore[pVulkanContext->swapChainImages.size() + 1] );

        stackAllocator.Setup(allocatorSize);
    }
    { // Allocate on stackAllocator
        assert(aFences.Allocate(pVulkanContext->MAX_FRAMES_IN_FLIGHT));
        assert(aImageAvailableSemaphores.Allocate(pVulkanContext->swapChainImages.size() + 1)); // We need one extra slot later
        assert(aSubmitDatas.Allocate(sceneStackSize));
        assert(aSubmitInfos.Allocate(sceneStackSize));
        assert(aCommandPools.Allocate(sceneStackSize));
        assert(aaLayerSemaphores.Allocate(pVulkanContext->swapChainImages.size(), sceneStackSize));
    }
    { // Allocate all semaphore and fences
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t index = 0; index < aImageAvailableSemaphores.capacity(); ++index)
        {
            if (vkCreateSemaphore(pVulkanContext->virtualGPU, &semaphoreCreateInfo, nullptr, aImageAvailableSemaphores.ptrAt(index))
                != VK_SUCCESS)
                CHIN_LOG_CRITICAL("Failed to create semaphore!");
        }

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t index = 0; index < aFences.capacity(); ++index)
        {
            if (vkCreateFence(pVulkanContext->virtualGPU, &fenceCreateInfo, nullptr, aFences.ptrAt(index))
                != VK_SUCCESS)
                CHIN_LOG_CRITICAL("Failed to create fence!");
        }

        for (uint32_t i = 0; i < aaLayerSemaphores.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aaLayerSemaphores.secondOrderCapacity(); ++j)
            {
                if (vkCreateSemaphore(pVulkanContext->virtualGPU, &semaphoreCreateInfo, nullptr,
                    aaLayerSemaphores.ptrAt(i, j))
                    != VK_SUCCESS)
                    CHIN_LOG_CRITICAL("Failed to allocate Semaphore!");
            }
        }

        for (uint32_t index = 0; index < aCommandPools.capacity(); ++index)
        {
             ChinVulkan::CreateCommandPool(*pVulkanContext, aCommandPools.ptrAt(index));
        }
    }
    { // Allocate Command Buffers
        // We only support one command buffer per scene for now, but every scene must have as many command buffers as we can have frames in flight
        const uint32_t bufferAllocatorSize = sizeof(VkCommandBuffer[sceneStackSize][pVulkanContext->MAX_FRAMES_IN_FLIGHT]);
        cmdBufferAllocator.Setup(bufferAllocatorSize);

        assert(aaCmdBuffers.Allocate(sceneStackSize, pVulkanContext->MAX_FRAMES_IN_FLIGHT));
    }
    {
        for (uint32_t i = 0; i < aaCmdBuffers.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aaCmdBuffers.secondOrderCapacity(); ++j)
            {
                ChinVulkan::CreateCommandBuffer(*pVulkanContext, aaCmdBuffers.ptrAt(i, j), aCommandPools.ptrAt(i));
            }
        }

        // Give out pointers to every scene
        uint32_t index = 0;
        for (auto& scene : Application::App::Get().GetSceneStack())
        {
            scene->standardCmdBufferArray = aaCmdBuffers.ptrAt(index, 0);
            ++index;
        }
    }
}

bool Chinstrap::Renderer::BeginFrame(const uint32_t currentFrame, RenderContext &renderContext)
{
    vkWaitForFences(renderContext.pVulkanContext->virtualGPU, 1, renderContext.aFences.ptrAt(currentFrame), VK_TRUE, UINT64_MAX);

    VkResult acquireImageResult = vkAcquireNextImageKHR(renderContext.pVulkanContext->virtualGPU, renderContext.pVulkanContext->swapChain, UINT64_MAX,
                      // Provide the extra semaphore slot at the end of the array, which is guaranteed to be unused
                      *renderContext.aImageAvailableSemaphores.ptrAt(renderContext.aImageAvailableSemaphores.capacity()-1),
                      VK_NULL_HANDLE, &RenderContext::imageIndex);

    // Swap the extra slot with the semaphore that was last used for this Index, so that the extra slot is guaranteed to be available
    std::swap(*renderContext.aImageAvailableSemaphores.ptrAt(RenderContext::imageIndex),
        *renderContext.aImageAvailableSemaphores.ptrAt(renderContext.aImageAvailableSemaphores.capacity()-1));

    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) [[unlikely]]
    {
        ChinVulkan::RecreateSwapChain(Application::App::Get().frame);
        return true;
    }
    return false;
}

void Chinstrap::Renderer::SubmitDrawData(const uint32_t currentFrame, RenderContext &renderContext)
{
    vkResetFences(renderContext.pVulkanContext->virtualGPU, 1, renderContext.aFences.ptrAt(currentFrame));

    *renderContext.aSubmitDatas.ptrAt(0) = {
        *renderContext.aImageAvailableSemaphores.ptrAt(RenderContext::imageIndex),
        *renderContext.aaLayerSemaphores.ptrAt(RenderContext::imageIndex, 0),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    for (uint32_t index = 1; index < renderContext.pSceneStack->size(); ++index)
    {
        *renderContext.aSubmitDatas.ptrAt(index) = {
            *renderContext.aaLayerSemaphores.ptrAt(RenderContext::imageIndex, index-1),
            *renderContext.aaLayerSemaphores.ptrAt(RenderContext::imageIndex, index),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
    }

    for (uint32_t index = 0; index < renderContext.pSceneStack->size(); ++index)
    {
        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &(renderContext.aSubmitDatas.ptrAt(index))->startSemaphore[0];
        submit.pWaitDstStageMask = renderContext.aSubmitDatas.ptrAt(index)->dstStageMask;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = renderContext.aaCmdBuffers.ptrAt(index, currentFrame);
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderContext.aSubmitDatas.ptrAt(index)->signalSemaphore[0];

        *renderContext.aSubmitInfos.ptrAt(index) = submit;
    }
}

void Chinstrap::Renderer::RenderFrame(const uint32_t currentFrame, RenderContext &renderContext)
{
    if (vkQueueSubmit(renderContext.pVulkanContext->graphicsQueue, renderContext.aSubmitInfos.capacity(),
        renderContext.aSubmitInfos.data(), *renderContext.aFences.ptrAt(currentFrame))
        != VK_SUCCESS) [[unlikely]]
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {renderContext.pVulkanContext->swapChain};
    VkSemaphore signalSemaphores[] = {*renderContext.aaLayerSemaphores.ptrAt(RenderContext::imageIndex, renderContext.pSceneStack->size()-1)};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &RenderContext::imageIndex;
    VkResult queuePresentResult = vkQueuePresentKHR(renderContext.pVulkanContext->graphicsQueue, &presentInfo);

    if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR
        || queuePresentResult == VK_SUBOPTIMAL_KHR
        || renderContext.pVulkanContext->swapChainInadequate) [[unlikely]]
    {
        renderContext.pVulkanContext->swapChainInadequate = false;
        ChinVulkan::RecreateSwapChain(Application::App::Get().frame);
    }

    renderContext.pVulkanContext->currentFrame = (currentFrame + 1) % renderContext.pVulkanContext->MAX_FRAMES_IN_FLIGHT;
}

void Chinstrap::Renderer::SetupSceneCmdBuffers(uint8_t sceneIndex, RenderContext &renderContext)
{
    // Hand out pointer to beginning of default array
    Application::App::Get().GetSceneStack().at(sceneIndex)->standardCmdBufferArray =
        renderContext.aaCmdBuffers.ptrAt(sceneIndex, 0);
}

void Chinstrap::Renderer::RenderContext::Destroy()
{
    vkDeviceWaitIdle(pVulkanContext->virtualGPU);

    {
        for (uint32_t index = 0; index < aCommandPools.capacity(); ++index)
        {
            vkDestroyCommandPool(pVulkanContext->virtualGPU, *aCommandPools.ptrAt(index), nullptr);
        }
        cmdBufferAllocator.Cleanup();
    }

    { // Cleanup all stackAllocator allocations
        for (uint32_t index = 0; index < aImageAvailableSemaphores.capacity(); ++index)
        {
            vkDestroySemaphore(pVulkanContext->virtualGPU, *aImageAvailableSemaphores.ptrAt(index), nullptr);
        }
        for (uint32_t index = 0; index < aFences.capacity(); ++index)
        {
            vkDestroyFence(pVulkanContext->virtualGPU, *aFences.ptrAt(index), nullptr);
        }
        for (uint32_t i = 0; i < aaLayerSemaphores.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aaLayerSemaphores.secondOrderCapacity(); ++j)
            {
                vkDestroySemaphore(pVulkanContext->virtualGPU, *aaLayerSemaphores.ptrAt(i, j), nullptr);
            }
        }

        stackAllocator.Cleanup();
    }

}
