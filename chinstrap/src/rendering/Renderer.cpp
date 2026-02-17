#include "Renderer.h"

#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"
#include <vulkan/vulkan_core.h>

#include "../ops/DevInterface.h"

namespace Chinstrap::Renderer
{
    namespace
    {
        struct RenderContext
        {
            ChinVulkan::Restaurant *restaurant = nullptr;
            ChinVulkan::Restaurant *imGuiRestaurant = nullptr;
            ChinVulkan::VulkanContext* vulkanContext = nullptr;

            std::function<void()> imGuiRequests = nullptr;

            uint32_t imageIndex = 0;
            std::vector<ChinVulkan::FrameSync> frameSyncs;
        };
        RenderContext context;
    }
}

void Chinstrap::Renderer::Setup()
{
    context.imGuiRestaurant = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    context.restaurant      = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    context.vulkanContext = &Application::App::Get().frame->vulkanContext;

    ChinVulkan::CreateSyncObjects(*context.vulkanContext, context.frameSyncs);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (auto &frameSync : context.frameSyncs)
    {
        uint32_t size = Application::App::Get().sceneStack.size();
        CHIN_LOG_INFO_VULKAN_F("Size for layerSemaphores: {}", size);

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

void Chinstrap::Renderer::SetDevInterface(void(*lambda)())
{
    assert(context.imGuiRequests == nullptr);

    context.imGuiRequests = lambda;
}

bool Chinstrap::Renderer::BeginFrame(const uint32_t currentFrame)
{

    vkWaitForFences(context.vulkanContext->virtualGPU, 1, &context.frameSyncs.at(currentFrame).inFlightFence, VK_TRUE, UINT64_MAX);

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
    vkResetCommandBuffer(context.restaurant->commandBuffers[currentFrame], 0);
    ChinVulkan::ExampleRecordCommandBuffer(context.restaurant->commandBuffers[currentFrame], context.imageIndex, *context.restaurant,
                                           context.restaurant->materials.front(), *context.vulkanContext);

    DevInterface::Render(context.imGuiRequests);
    vkResetCommandBuffer(context.imGuiRestaurant->commandBuffers[currentFrame], 0);
    ChinVulkan::RecordImGUICommandBuffer(context.imGuiRestaurant->commandBuffers[currentFrame],
                                           context.vulkanContext->defaultImageViews[context.imageIndex], *context.imGuiRestaurant);

    context.frameSyncs[currentFrame].submitData.at(0) = {
        context.frameSyncs[currentFrame].imageAvailableSemaphore,
        context.frameSyncs[currentFrame].layerSemaphores.front(),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    VkSubmitInfo triangleSubmit = {};
    triangleSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    triangleSubmit.waitSemaphoreCount = 1;
    triangleSubmit.pWaitSemaphores = context.frameSyncs[currentFrame].submitData.at(0).startSemaphore;
    triangleSubmit.pWaitDstStageMask = context.frameSyncs[currentFrame].submitData.at(0).waitStageMask;
    triangleSubmit.commandBufferCount = 1;
    triangleSubmit.pCommandBuffers = &context.restaurant->commandBuffers[currentFrame];
    triangleSubmit.signalSemaphoreCount = 1;
    triangleSubmit.pSignalSemaphores = context.frameSyncs[currentFrame].submitData.at(0).signalSemaphore;
    context.frameSyncs[currentFrame].submitInfos.at(0) = triangleSubmit;

    context.frameSyncs[currentFrame].submitData.at(1) = {
        context.frameSyncs[currentFrame].layerSemaphores.front(),
        context.frameSyncs[currentFrame].renderFinishedSemaphore,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    VkSubmitInfo imGuiSubmit = {};
    imGuiSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    imGuiSubmit.waitSemaphoreCount = 1;
    imGuiSubmit.pWaitSemaphores = context.frameSyncs[currentFrame].submitData.at(1).startSemaphore;
    imGuiSubmit.pWaitDstStageMask = context.frameSyncs[currentFrame].submitData.at(1).waitStageMask;
    imGuiSubmit.commandBufferCount = 1;
    imGuiSubmit.pCommandBuffers = &context.imGuiRestaurant->commandBuffers[currentFrame];
    imGuiSubmit.signalSemaphoreCount = 1;
    imGuiSubmit.pSignalSemaphores = context.frameSyncs[currentFrame].submitData.at(1).signalSemaphore;
    context.frameSyncs[currentFrame].submitInfos.at(1) = imGuiSubmit;
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
    VkSemaphore signalSemaphores[] = {context.frameSyncs[currentFrame].renderFinishedSemaphore};

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
    delete context.restaurant;
    delete context.imGuiRestaurant;

    for (size_t i = 0; i < vulkanContext.MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkanContext.virtualGPU, context.frameSyncs[i].imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(vulkanContext.virtualGPU, context.frameSyncs[i].renderFinishedSemaphore, nullptr);
        vkDestroyFence(vulkanContext.virtualGPU, context.frameSyncs[i].inFlightFence, nullptr);
        for (auto &layerSemaphore : context.frameSyncs[i].layerSemaphores)
        {
            vkDestroySemaphore(vulkanContext.virtualGPU, layerSemaphore, nullptr);
        }
    }
}
