#include "Renderer.h"

// This Renderer Setup ist temporary to develop and test underlying Vulkan systems

#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"
#include <vulkan/vulkan_core.h>

#include "../ops/DevInterface.h"
#include "spdlog/fmt/bundled/compile.h"

namespace Chinstrap::Renderer
{
    namespace
    {
        struct RenderContext
        {
            ChinVulkan::Restaurant *restaurant = nullptr;
            ChinVulkan::Restaurant *imGuiRestaurant = nullptr;
            ChinVulkan::VulkanContext* vulkanContext = nullptr;
        };
        RenderContext context;
    }
}

void Chinstrap::Renderer::Shutdown(const ChinVulkan::VulkanContext &vulkanContext)
{
    vkDeviceWaitIdle(vulkanContext.virtualGPU);
    delete context.restaurant;
    delete context.imGuiRestaurant;
}

void Chinstrap::Renderer::Setup()
{
    context.imGuiRestaurant = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    context.restaurant      = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    context.vulkanContext = &Application::App::Get().frame->vulkanContext;
}

void Chinstrap::Renderer::DrawFrame()
{
    uint32_t imageIndex;
    vkWaitForFences(context.vulkanContext->virtualGPU, 1, &context.vulkanContext->inFlightFences[context.vulkanContext->currentFrame], VK_TRUE, UINT64_MAX);

    VkResult acquireImageResult = vkAcquireNextImageKHR(context.vulkanContext->virtualGPU, context.vulkanContext->swapChain,
                      UINT64_MAX, context.vulkanContext->imageAvailableSemaphores[context.vulkanContext->currentFrame],
                      VK_NULL_HANDLE, &imageIndex);
    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
        return;
    }
    // Reset the fences only when sure that work will be submited to the GPU, otherwise we end up in a deadlock when returning after resetting
    vkResetFences(context.vulkanContext->virtualGPU, 1, &context.vulkanContext->inFlightFences[context.vulkanContext->currentFrame]);

    vkResetCommandBuffer(context.restaurant->commandBuffers[context.vulkanContext->currentFrame], 0);
    ChinVulkan::ExampleRecordCommandBuffer(context.restaurant->commandBuffers[context.vulkanContext->currentFrame], imageIndex, *context.restaurant,
                                           context.restaurant->materials.front(), *context.vulkanContext);

    Chinstrap::DevInterface::Render([]()
    {
        Chinstrap::DevInterface::ContextInfo(0.7f, 0.0f);
        Chinstrap::DevInterface::PerformanceInfo(0.0f, 0.0f);
    });
    vkResetCommandBuffer(context.imGuiRestaurant->commandBuffers[context.vulkanContext->currentFrame], 0);
    ChinVulkan::RecordImGUICommandBuffer(context.imGuiRestaurant->commandBuffers[context.vulkanContext->currentFrame],
                                           context.vulkanContext->defaultImageViews[imageIndex], *context.imGuiRestaurant);

    VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore waitSemaphores[] = {context.vulkanContext->imageAvailableSemaphores[context.vulkanContext->currentFrame]};
    VkSemaphore firstStageSemaphores[] = {context.vulkanContext->firstStageFinishedSemaphores[context.vulkanContext->currentFrame]};
    VkSemaphore signalSemaphores[] = {context.vulkanContext->renderFinishedSemaphores[context.vulkanContext->currentFrame]};

    VkSubmitInfo triangleSubmit = {};
    triangleSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    triangleSubmit.waitSemaphoreCount = 1;
    triangleSubmit.pWaitSemaphores = waitSemaphores;
    triangleSubmit.pWaitDstStageMask = waitStages;
    triangleSubmit.commandBufferCount = 1;
    triangleSubmit.pCommandBuffers = &context.restaurant->commandBuffers[context.vulkanContext->currentFrame];
    triangleSubmit.signalSemaphoreCount = 1;
    triangleSubmit.pSignalSemaphores = firstStageSemaphores;

    VkSubmitInfo imGuiSubmit = {};
    imGuiSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    imGuiSubmit.waitSemaphoreCount = 1;
    imGuiSubmit.pWaitSemaphores = firstStageSemaphores;
    imGuiSubmit.pWaitDstStageMask = waitStages;
    imGuiSubmit.commandBufferCount = 1;
    imGuiSubmit.pCommandBuffers = &context.imGuiRestaurant->commandBuffers[context.vulkanContext->currentFrame];
    imGuiSubmit.signalSemaphoreCount = 1;
    imGuiSubmit.pSignalSemaphores = signalSemaphores;
    VkSubmitInfo submitInfos[] = { triangleSubmit, imGuiSubmit };

    if (vkQueueSubmit(context.vulkanContext->graphicsQueue, 2, submitInfos, context.vulkanContext->inFlightFences[context.vulkanContext->currentFrame]) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {context.vulkanContext->swapChain};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    VkResult queuePresentResult = vkQueuePresentKHR(context.vulkanContext->graphicsQueue, &presentInfo);

    if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR
        || queuePresentResult == VK_SUBOPTIMAL_KHR
        || context.vulkanContext->frameResized)
    {
        context.vulkanContext->frameResized = false;
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
    }

    context.vulkanContext->currentFrame = (context.vulkanContext->currentFrame + 1) % context.vulkanContext->MAX_FRAMES_IN_FLIGHT;
}
