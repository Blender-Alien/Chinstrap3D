#include "Renderer.h"
#include "VulkanFunctions.h"
#include "VulkanData.h"
#include "../Application.h"

namespace Chinstrap::Renderer
{
    /* TEMPORARY HACK */
    namespace
    {
        ChinVulkan::Restaurant* restaurant = nullptr;
    }
    void Setup()
    {
        restaurant = new ChinVulkan::Restaurant(Application::App::Get().frame->vulkanContext);
    }
    void Delete()
    {
        delete restaurant;
    }
    /* END OF HACK */
    void DrawFrame()
    {
        ChinVulkan::VulkanContext context = Application::App::Get().frame->vulkanContext;
        vkWaitForFences(context.virtualGPU, 1, &context.inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(context.virtualGPU, 1, &context.inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(context.virtualGPU, context.swapChain, UINT64_MAX, context.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(restaurant->commandBuffers.front(), 0);

        ChinVulkan::ExampleRecordCommandBuffer(restaurant->commandBuffers.front(), imageIndex, *restaurant, restaurant->materials.front(), context);
        VkSubmitInfo submitinfo = {};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { context.imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = waitSemaphores;
        submitinfo.pWaitDstStageMask = waitStages;

        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &restaurant->commandBuffers.front();

        VkSemaphore signalSemaphores[] = { context.renderFinishedSemaphore };
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(context.graphicsQueue, 1, &submitinfo, context.inFlightFence) != VK_SUCCESS)
        {
            CHIN_LOG_ERROR_VULKAN("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { context.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(context.graphicsQueue, &presentInfo);
    }
}
