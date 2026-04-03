#include "TestMenuScene.h"

#include "chinstrap/src/ops/Logging.h"

#include "TestGLScene.h"

#include "GLFW/glfw3.h"

void Game::TestMenuScene::OnBegin()
{
    using namespace Chinstrap;
}

void Game::TestMenuScene::OnUpdate(float deltaTime)
{
}

void Game::TestMenuScene::OnRender(uint32_t currentFrame)
{
    /* Example of directly using vulkan calls in OnRender
     * We still want to abstract some of this stuff into ChinVulkan::
     * function calls that either are ALWAYS the same,
     * or require dynamic info/logic like the viewport
     */

    using namespace Chinstrap;
    {
        auto targetCommandBuffer = standardCmdBufferArray[currentFrame];
        auto vulkanContext = Application::App::GetVulkanContext();
        //auto pipeline = material->pipeline;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkResetCommandBuffer(targetCommandBuffer, 0);
        if (vkBeginCommandBuffer(targetCommandBuffer, &beginInfo) != VK_SUCCESS)
        {
            CHIN_LOG_ERROR_VULKAN("Failed to begin recording command buffer!");
        }
        {
            VkImageMemoryBarrier waitImageMemoryBarrier = {};
            waitImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            waitImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            waitImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            waitImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            waitImageMemoryBarrier.image = vulkanContext.swapChainImages.at(Renderer::RenderContext::GetImageIndex());
            waitImageMemoryBarrier.subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };
            vkCmdPipelineBarrier(
                targetCommandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &waitImageMemoryBarrier
            );
        }
        {
            VkRenderingAttachmentInfo colorAttachmentInfo = {};
            colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkClearValue clearColor = {{{0.02f, 0.1f, 0.05f, 1.0f}}};
            colorAttachmentInfo.clearValue = clearColor;
            colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentInfo.imageView = vulkanContext.defaultImageViews.at(Renderer::RenderContext::GetImageIndex());
            VkRenderingInfo renderInfo = {};
            renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            renderInfo.layerCount = 1;
            renderInfo.colorAttachmentCount = 1;
            renderInfo.pColorAttachments = &colorAttachmentInfo;
            renderInfo.renderArea.offset = {0, 0};
            renderInfo.renderArea.extent = vulkanContext.swapChainExtent;

            if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
                vulkanContext.PFN_vkCmdBeginRenderingKHR(targetCommandBuffer, &renderInfo);
            else
                vkCmdBeginRendering(targetCommandBuffer, &renderInfo);

            // Get pipeline from material resource system
            //vkCmdBindPipeline(targetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }
        {
            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(vulkanContext.swapChainExtent.width);
            viewport.height = static_cast<float>(vulkanContext.swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(targetCommandBuffer, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.offset = {0, 0};
            scissor.extent = vulkanContext.swapChainExtent;
            vkCmdSetScissor(targetCommandBuffer, 0, 1, &scissor);
        }
    }

    ChinVulkan::EndRendering(standardCmdBufferArray[currentFrame], Application::App::GetVulkanContext(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void Game::TestMenuScene::OnKeyPress(Chinstrap::Event& event)
{
    switch (event.eventData.KeyPressed.keyCode)
    {
    case GLFW_KEY_HOME:
        if (!event.eventData.KeyPressed.repeat)
        {
            QueueChangeToScene<TestGLScene>();
            event.handled = true;
        }

    case GLFW_KEY_1:
        if (!event.eventData.KeyPressed.repeat)
        {
            CHIN_LOG_INFO("We're in the TestGLScene!!");
        }
    default: ;
    }
}

void Game::TestMenuScene::OnEvent(Chinstrap::Event &event)
{
    switch (event.type)
    {
    case Chinstrap::EventType::KeyPressed:
        OnKeyPress(event);
        break;
    default: ;
    }
}