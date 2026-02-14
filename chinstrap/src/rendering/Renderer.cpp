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
        ChinVulkan::Restaurant *restaurant = nullptr;
        ChinVulkan::Restaurant *imGuiRestaurant = nullptr;

        /* Idea: We need these variables in the DrawFrame() function,
         * but we don't want to allocate and deallocate them to the stack every frame 
         * TODO: Profile if this actually saves performance */
        struct DrawStuff
        {
            uint32_t imageIndex;
            ChinVulkan::VulkanContext* context = nullptr;
            VkResult acquireImageResult;
            VkResult queuePresentResult;
            VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        };

        DrawStuff drawStuff;
    }
}

void Chinstrap::Renderer::Shutdown(const ChinVulkan::VulkanContext &context)
{
    vkDeviceWaitIdle(context.virtualGPU);
    delete restaurant;
    delete imGuiRestaurant;
}

void Chinstrap::Renderer::Setup()
{
    imGuiRestaurant = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    restaurant      = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);

    drawStuff.context = &Application::App::Get().frame->vulkanContext;
}

void Chinstrap::Renderer::DrawFrame()
{
    vkWaitForFences(drawStuff.context->virtualGPU, 1, &drawStuff.context->inFlightFences[drawStuff.context->currentFrame], VK_TRUE, UINT64_MAX);

    drawStuff.acquireImageResult = vkAcquireNextImageKHR(drawStuff.context->virtualGPU, drawStuff.context->swapChain,
                      UINT64_MAX, drawStuff.context->imageAvailableSemaphores[drawStuff.context->currentFrame],
                      VK_NULL_HANDLE, &drawStuff.imageIndex);
    if (drawStuff.acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
        return;
    }
    // Reset the fences only when sure that work will be submited to the GPU, otherwise we end up in a deadlock when returning after resetting
    vkResetFences(drawStuff.context->virtualGPU, 1, &drawStuff.context->inFlightFences[drawStuff.context->currentFrame]);

    vkResetCommandBuffer(restaurant->commandBuffers[drawStuff.context->currentFrame], 0);
    ChinVulkan::ExampleRecordCommandBuffer(restaurant->commandBuffers[drawStuff.context->currentFrame], drawStuff.imageIndex, *restaurant,
                                           restaurant->materials.front(), *drawStuff.context);

    Chinstrap::DevInterface::Render([]()
    {
        Chinstrap::DevInterface::ContextInfo(0.7f, 0.0f);
        Chinstrap::DevInterface::PerformanceInfo(0.0f, 0.0f);
    });
    vkResetCommandBuffer(imGuiRestaurant->commandBuffers[drawStuff.context->currentFrame], 0);
    ChinVulkan::RecordImGUICommandBuffer(imGuiRestaurant->commandBuffers[drawStuff.context->currentFrame],
                                           drawStuff.context->defaultImageViews[drawStuff.imageIndex], *imGuiRestaurant, imGuiRestaurant->materials.front());

    VkSemaphore waitSemaphores[] = {drawStuff.context->imageAvailableSemaphores[drawStuff.context->currentFrame]};
    VkSemaphore firstStageSemaphores[] = {drawStuff.context->firstStageFinishedSemaphores[drawStuff.context->currentFrame]};
    VkSemaphore signalSemaphores[] = {drawStuff.context->renderFinishedSemaphores[drawStuff.context->currentFrame]};

    VkSubmitInfo triangleSubmit = {};
    triangleSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    triangleSubmit.waitSemaphoreCount = 1;
    triangleSubmit.pWaitSemaphores = waitSemaphores;
    triangleSubmit.pWaitDstStageMask = drawStuff.waitStages;
    triangleSubmit.commandBufferCount = 1;
    triangleSubmit.pCommandBuffers = &restaurant->commandBuffers[drawStuff.context->currentFrame];
    triangleSubmit.signalSemaphoreCount = 1;
    triangleSubmit.pSignalSemaphores = firstStageSemaphores;

    VkSubmitInfo imGuiSubmit = {};
    imGuiSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    imGuiSubmit.waitSemaphoreCount = 1;
    imGuiSubmit.pWaitSemaphores = firstStageSemaphores;
    imGuiSubmit.pWaitDstStageMask = drawStuff.waitStages;
    imGuiSubmit.commandBufferCount = 1;
    imGuiSubmit.pCommandBuffers = &imGuiRestaurant->commandBuffers[drawStuff.context->currentFrame];
    imGuiSubmit.signalSemaphoreCount = 1;
    imGuiSubmit.pSignalSemaphores = signalSemaphores;
    VkSubmitInfo submitInfos[] = { triangleSubmit, imGuiSubmit };

    if (vkQueueSubmit(drawStuff.context->graphicsQueue, 2, submitInfos, drawStuff.context->inFlightFences[drawStuff.context->currentFrame]) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {drawStuff.context->swapChain};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &drawStuff.imageIndex;
    drawStuff.queuePresentResult = vkQueuePresentKHR(drawStuff.context->graphicsQueue, &presentInfo);

    if (drawStuff.queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR 
        || drawStuff.queuePresentResult == VK_SUBOPTIMAL_KHR 
        || drawStuff.context->frameResized)
    {
        drawStuff.context->frameResized = false;
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
    }

    drawStuff.context->currentFrame = (drawStuff.context->currentFrame + 1) % drawStuff.context->MAX_FRAMES_IN_FLIGHT;
}
