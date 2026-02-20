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

        VkSemaphore* ptr = nullptr;
        for (uint32_t index = 0; index < aImageAvailableSemaphores.capacity(); ++index)
        {
            ptr = (VkSemaphore*)aImageAvailableSemaphores.at(index);
            if (vkCreateSemaphore(pVulkanContext->virtualGPU, &semaphoreCreateInfo, nullptr, (VkSemaphore*)aImageAvailableSemaphores.at(index))
                != VK_SUCCESS)
                CHIN_LOG_CRITICAL("Failed to create semaphore!");
        }

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t index = 0; index < aFences.capacity(); ++index)
        {
            if (vkCreateFence(pVulkanContext->virtualGPU, &fenceCreateInfo, nullptr, (VkFence*)aFences.at(index))
                != VK_SUCCESS)
                CHIN_LOG_CRITICAL("Failed to create fence!");
        }

        for (uint32_t i = 0; i < aaLayerSemaphores.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aaLayerSemaphores.secondOrderCapacity(); ++j)
            {
                if (vkCreateSemaphore(pVulkanContext->virtualGPU, &semaphoreCreateInfo, nullptr,
                    (VkSemaphore*)aaLayerSemaphores.at(i, j))
                    != VK_SUCCESS)
                    CHIN_LOG_CRITICAL("Failed to allocate Semaphore!");
            }
        }

        for (uint32_t index = 0; index < aCommandPools.capacity(); ++index)
        {
             ChinVulkan::ExampleCreateCommandPool(*pVulkanContext, (VkCommandPool*)aCommandPools.at(index));
        }
    }
    { // Allocate Command Buffers
        // We only support one command buffer per scene for now, but every scene must have as many command buffers as we can have frames in flight
        const uint32_t bufferAllocatorSize = sizeof(VkCommandBuffer[sceneStackSize][pVulkanContext->MAX_FRAMES_IN_FLIGHT]);
        cmdBufferAllocator.Setup(bufferAllocatorSize);

        assert(aCmdBuffers.Allocate(sceneStackSize, pVulkanContext->MAX_FRAMES_IN_FLIGHT));
    }
    {
        for (uint32_t i = 0; i < aCmdBuffers.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aCmdBuffers.secondOrderCapacity(); ++j)
            {
                ChinVulkan::ExampleCreateCommandBuffer(*pVulkanContext, (VkCommandBuffer*)aCmdBuffers.at(i, j), (VkCommandPool*)aCommandPools.at(i));
            }
        }

        // Give out pointers to every scene
        uint32_t index = 0;
        for (auto& scene : Application::App::Get().GetSceneStack())
        {
            scene->standardCmdBufferArray = (VkCommandBuffer*)aCmdBuffers.at(index, 0);
            ++index;
        }
    }
}

bool Chinstrap::Renderer::BeginFrame(const uint32_t currentFrame, RenderContext &renderContext)
{
    vkWaitForFences(renderContext.pVulkanContext->virtualGPU, 1, (VkFence*)renderContext.aFences.at(currentFrame), VK_TRUE, UINT64_MAX);

    VkResult acquireImageResult = vkAcquireNextImageKHR(renderContext.pVulkanContext->virtualGPU, renderContext.pVulkanContext->swapChain, UINT64_MAX,
                      // Provide the extra semaphore slot at the end of the array, which is guaranteed to be unused
                      *(VkSemaphore*)renderContext.aImageAvailableSemaphores.at(renderContext.aImageAvailableSemaphores.capacity()-1),
                      VK_NULL_HANDLE, &RenderContext::imageIndex);

    // Swap the extra slot with the semaphore that was last used for this Index, so that the extra slot is guaranteed to be available
    std::swap(*(VkSemaphore*)renderContext.aImageAvailableSemaphores.at(RenderContext::imageIndex),
        *(VkSemaphore*)renderContext.aImageAvailableSemaphores.at(renderContext.aImageAvailableSemaphores.capacity()-1));

    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) [[unlikely]]
    {
        ChinVulkan::RecreateSwapChain(Application::App::Get().frame);
        return true;
    }
    return false;
}

void Chinstrap::Renderer::SubmitDrawData(const uint32_t currentFrame, RenderContext &renderContext)
{
    vkResetFences(renderContext.pVulkanContext->virtualGPU, 1, (VkFence*)renderContext.aFences.at(currentFrame));

    *(ChinVulkan::SubmitData*)renderContext.aSubmitDatas.at(0) = {
        *(VkSemaphore*)renderContext.aImageAvailableSemaphores.at(RenderContext::imageIndex),
        *(VkSemaphore*)renderContext.aaLayerSemaphores.at(RenderContext::imageIndex, 0),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    for (uint32_t index = 1; index < renderContext.pSceneStack->size(); ++index)
    {
        *(ChinVulkan::SubmitData*)renderContext.aSubmitDatas.at(index) = {
            *(VkSemaphore*)renderContext.aaLayerSemaphores.at(RenderContext::imageIndex, index-1),
            *(VkSemaphore*)renderContext.aaLayerSemaphores.at(RenderContext::imageIndex, index),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
    }

    for (uint32_t index = 0; index < renderContext.pSceneStack->size(); ++index)
    {
        /*
        {
             DevInterface::Render([]()
             {
                 DevInterface::ContextInfo(0.7f, 0.0f);
                 DevInterface::PerformanceInfo(0.0f, 0.0f);
             });
             vkResetCommandBuffer(context.restaurant[index].commandBuffers[currentFrame], 0);
             ChinVulkan::RecordImGUICommandBuffer(context.restaurant[index].commandBuffers[currentFrame], context.pVulkanContext->defaultImageViews[context.imageIndex],
                                                 context.restaurant[index], context.frameSyncs[currentFrame].submitData.at(index).waitStageMask[0], context.imageIndex, *context.pVulkanContext);
        }
        */
        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &reinterpret_cast<ChinVulkan::SubmitData*>(renderContext.aSubmitDatas.at(index))->startSemaphore[0];
        submit.pWaitDstStageMask = reinterpret_cast<ChinVulkan::SubmitData*>(renderContext.aSubmitDatas.at(index))->dstStageMask;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = (VkCommandBuffer*)renderContext.aCmdBuffers.at(index, currentFrame);
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &reinterpret_cast<ChinVulkan::SubmitData*>(renderContext.aSubmitDatas.at(index))->signalSemaphore[0];

        *(VkSubmitInfo*)renderContext.aSubmitInfos.at(index) = submit;
    }
}

void Chinstrap::Renderer::RenderFrame(const uint32_t currentFrame, RenderContext &renderContext)
{
    if (vkQueueSubmit(renderContext.pVulkanContext->graphicsQueue, renderContext.aSubmitInfos.capacity(),
        (VkSubmitInfo*)renderContext.aSubmitInfos.data(), *(VkFence*)renderContext.aFences.at(currentFrame))
        != VK_SUCCESS) [[unlikely]]
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {renderContext.pVulkanContext->swapChain};
    VkSemaphore signalSemaphores[] = {*(VkSemaphore*)renderContext.aaLayerSemaphores.at(RenderContext::imageIndex, renderContext.pSceneStack->size()-1)};

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

void Chinstrap::Renderer::RenderContext::Destroy()
{
    vkDeviceWaitIdle(pVulkanContext->virtualGPU);

    {
        for (uint32_t index = 0; index < aCommandPools.capacity(); ++index)
        {
            vkDestroyCommandPool(pVulkanContext->virtualGPU, *(VkCommandPool*)aCommandPools.at(index), nullptr);
        }
        cmdBufferAllocator.Cleanup();
    }

    { // Cleanup all stackAllocator allocations
        for (uint32_t index = 0; index < aImageAvailableSemaphores.capacity(); ++index)
        {
            vkDestroySemaphore(pVulkanContext->virtualGPU, *(VkSemaphore*)aImageAvailableSemaphores.at(index), nullptr);
        }
        for (uint32_t index = 0; index < aFences.capacity(); ++index)
        {
            vkDestroyFence(pVulkanContext->virtualGPU, *(VkFence*)aFences.at(index), nullptr);
        }
        for (uint32_t i = 0; i < aaLayerSemaphores.firstOrderCapacity(); ++i)
        {
            for (uint32_t j = 0; j < aaLayerSemaphores.secondOrderCapacity(); ++j)
            {
                vkDestroySemaphore(pVulkanContext->virtualGPU, *(VkSemaphore*)aaLayerSemaphores.at(i, j), nullptr);
            }
        }

        stackAllocator.Cleanup();
    }

}