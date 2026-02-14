#include "Renderer.h"

// This Renderer Setup ist temporary to develop and test underlying Vulkan systems

#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"
#include <vulkan/vulkan_core.h>

namespace Chinstrap::Renderer
{
    namespace
    {
        ChinVulkan::Restaurant *restaurant = nullptr;

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
}

void Chinstrap::Renderer::Setup()
{
    restaurant = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
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

    VkSemaphore waitSemaphores[] = {drawStuff.context->imageAvailableSemaphores[drawStuff.context->currentFrame]};
    VkSemaphore signalSemaphores[] = {drawStuff.context->renderFinishedSemaphores[drawStuff.context->currentFrame]};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = drawStuff.waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &restaurant->commandBuffers[drawStuff.context->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(drawStuff.context->graphicsQueue, 1, &submitInfo, drawStuff.context->inFlightFences[drawStuff.context->currentFrame]) != VK_SUCCESS)
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
