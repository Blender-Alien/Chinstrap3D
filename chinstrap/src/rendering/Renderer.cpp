#include "Renderer.h"

#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"
#include <vulkan/vulkan_core.h>

#include "../Scene.h"

/* TODO: Handle resizing of sceneStack at runtime with proper synchronization */
namespace Chinstrap::Renderer
{
    namespace
    {
        struct RenderContext
        {
            ChinVulkan::VulkanContext* vulkanContext = nullptr;
            std::vector<std::unique_ptr<Scene>>* pSceneStack = nullptr;

            std::vector<ChinVulkan::FrameSync> frameSyncs;
            uint32_t imageIndex = 0;
        };
        RenderContext context;
    }
}

void Chinstrap::Renderer::Setup()
{
    context.vulkanContext = &Application::App::Get().frame->vulkanContext;
    context.pSceneStack = &Application::App::Get().sceneStack;

    context.frameSyncs.resize(context.vulkanContext->MAX_FRAMES_IN_FLIGHT);

    for (auto &scene : Application::App::Get().sceneStack)
    {
        scene->restaurant.Initialize(context.vulkanContext);
    }

    ChinVulkan::CreateSyncObjects(*context.vulkanContext, context.frameSyncs);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (auto &frameSync : context.frameSyncs)
    {
        uint32_t size = Application::App::Get().sceneStack.size();

        frameSync.layerSemaphores.resize(size);
        frameSync.submitInfos.resize(size);
        frameSync.submitData.resize(size);

        for (auto &layer : frameSync.layerSemaphores)
        {
            if (vkCreateSemaphore(context.vulkanContext->virtualGPU, &semaphoreCreateInfo, nullptr, &layer) != VK_SUCCESS)
            {
                CHIN_LOG_CRITICAL_VULKAN("Failed to create layer semaphore!");
            }
        }
    }
}

bool Chinstrap::Renderer::BeginFrame(const uint32_t currentFrame)
{
    vkWaitForFences(context.vulkanContext->virtualGPU, 1, &context.frameSyncs[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);

    VkResult acquireImageResult = vkAcquireNextImageKHR(context.vulkanContext->virtualGPU, context.vulkanContext->swapChain,
                      UINT64_MAX, context.frameSyncs[currentFrame].imageAvailableSemaphore,
                      VK_NULL_HANDLE, &context.imageIndex);
    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR) [[unlikely]]
    {
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
        return false;
    }
    vkResetFences(context.vulkanContext->virtualGPU, 1, &context.frameSyncs[currentFrame].inFlightFence);
    return true;
}

void Chinstrap::Renderer::SubmitDrawData(const uint32_t currentFrame)
{
    context.frameSyncs[currentFrame].submitData.at(0) = {
        context.frameSyncs[currentFrame].imageAvailableSemaphore,
        context.frameSyncs[currentFrame].layerSemaphores.at(0),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    for (uint32_t index = 1; index < context.pSceneStack->size(); ++index)
    {
        context.frameSyncs[currentFrame].submitData.at(index) = {
            context.frameSyncs[currentFrame].layerSemaphores.at(index-1),
            context.frameSyncs[currentFrame].layerSemaphores.at(index),
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        };
    }

    for (uint32_t index = 0; index < context.pSceneStack->size(); ++index)
    {
        context.pSceneStack->at(index)->submitToRender(currentFrame, context.vulkanContext->defaultImageViews[context.imageIndex], *context.vulkanContext);

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = context.frameSyncs[currentFrame].submitData.at(index).startSemaphore;
        submit.pWaitDstStageMask = context.frameSyncs[currentFrame].submitData.at(index).waitStageMask;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &context.pSceneStack->at(index)->restaurant.commandBuffers[currentFrame];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = context.frameSyncs[currentFrame].submitData.at(index).signalSemaphore;

        context.frameSyncs[currentFrame].submitInfos.at(index) = submit;
    }
}

void Chinstrap::Renderer::RenderFrame(const uint32_t currentFrame)
{
    if (vkQueueSubmit(context.vulkanContext->graphicsQueue, context.frameSyncs[currentFrame].submitInfos.size(),
        context.frameSyncs[currentFrame].submitInfos.data(), context.frameSyncs[currentFrame].inFlightFence)
        != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {context.vulkanContext->swapChain};
    VkSemaphore signalSemaphores[] = {context.frameSyncs[currentFrame].layerSemaphores.at(context.pSceneStack->size()-1)};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &context.imageIndex;
    VkResult queuePresentResult = vkQueuePresentKHR(context.vulkanContext->graphicsQueue, &presentInfo);

    if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR
        || queuePresentResult == VK_SUBOPTIMAL_KHR
        || context.vulkanContext->swapChainInadequate)
    {
        context.vulkanContext->swapChainInadequate = false;
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
    }

    context.vulkanContext->currentFrame = (currentFrame + 1) % context.vulkanContext->MAX_FRAMES_IN_FLIGHT;
}

void Chinstrap::Renderer::Shutdown(const ChinVulkan::VulkanContext &vulkanContext)
{
    vkDeviceWaitIdle(vulkanContext.virtualGPU);

    for (auto& scene : Application::App::Get().sceneStack)
    {
        scene->restaurant.Cleanup();
    }

    for (size_t i = 0; i < vulkanContext.MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkanContext.virtualGPU, context.frameSyncs[i].imageAvailableSemaphore, nullptr);
        vkDestroyFence(vulkanContext.virtualGPU, context.frameSyncs[i].inFlightFence, nullptr);
        for (auto &layerSemaphore : context.frameSyncs[i].layerSemaphores)
        {
            vkDestroySemaphore(vulkanContext.virtualGPU, layerSemaphore, nullptr);
        }
    }
}