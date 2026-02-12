#include "Renderer.h"

// This Renderer Setup ist temporary to develop and test underlying Vulkan systems

#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"

namespace Chinstrap::Renderer
{
    namespace
    {
        ChinVulkan::Restaurant *restaurant = nullptr;
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
}

void Chinstrap::Renderer::DrawFrame()
{
    ChinVulkan::VulkanContext context = Application::App::Get().frame->vulkanContext;
    vkWaitForFences(context.virtualGPU, 1, &context.inFlightFences[context.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context.virtualGPU, 1, &context.inFlightFences[context.currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context.virtualGPU, context.swapChain, UINT64_MAX, context.imageAvailableSemaphores[context.currentFrame],
                          VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(restaurant->commandBuffers[context.currentFrame], 0);

    ChinVulkan::ExampleRecordCommandBuffer(restaurant->commandBuffers[context.currentFrame], imageIndex, *restaurant,
                                           restaurant->materials.front(), context);

    VkSemaphore waitSemaphores[] = {context.imageAvailableSemaphores[context.currentFrame]};
    VkSemaphore signalSemaphores[] = {context.renderFinishedSemaphores[context.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &restaurant->commandBuffers[context.currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, context.inFlightFences[context.currentFrame]) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {context.swapChain};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(context.graphicsQueue, &presentInfo);

    context.currentFrame = (context.currentFrame + 1) % context.MAX_FRAMES_IN_FLIGHT;
}
