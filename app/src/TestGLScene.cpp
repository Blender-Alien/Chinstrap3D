#include "TestGLScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"
#include "chinstrap/src/rendering/VulkanFunctions.h"
#include "chinstrap/src/rendering/VulkanData.h"
#include "chinstrap/src/rendering/Renderer.h"

#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TestMenuScene.h"
#include "chinstrap/src/Application.h"

void Game::TestGLScene::OnBegin()
{

}

void Game::TestGLScene::OnUpdate(float deltaTime)
{
}

//TODO: this is terrible
void Game::TestGLScene::OnRender()
{
    Chinstrap::ChinVulkan::VulkanContext context = Chinstrap::Application::App::Get().frame->vulkanContext;
    vkWaitForFences(context.virtualGPU, 1, &context.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(context.virtualGPU, 1, &context.inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context.virtualGPU, context.swapChain, UINT64_MAX, context.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(context.commandBuffer, 0);

    Chinstrap::ChinVulkan::RecordCommandBuffer(context.commandBuffer, context, imageIndex);
    VkSubmitInfo submitinfo = {};
    submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { context.imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitinfo.waitSemaphoreCount = 1;
    submitinfo.pWaitSemaphores = waitSemaphores;
    submitinfo.pWaitDstStageMask = waitStages;

    submitinfo.commandBufferCount = 1;
    submitinfo.pCommandBuffers = &context.commandBuffer;

    VkSemaphore signalSemaphores[] = { context.renderFinishedSemaphore };
    submitinfo.signalSemaphoreCount = 1;
    submitinfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context.graphicsQueue, 1, &submitinfo, context.inFlightFence) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR("[Vulkan] Failed to submit draw command buffer!");
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

bool Game::TestGLScene::OnKeyPress(const Chinstrap::KeyPressedEvent &event)
{
    switch (event.keyCode)
    {
        case GLFW_KEY_HOME:
            if (!event.repeat)
            {
                QueueChangeToScene<TestMenuScene>();
                return true;
            }

        case GLFW_KEY_1:
            if (!event.repeat)
            {
                CHIN_LOG_INFO("We're in the TestGLScene!!");
                return false;
            }

        default:
            return false;
    }
}

void Game::TestGLScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::KeyPressedEvent &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}
