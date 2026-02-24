#include "TestGLScene.h"

#include "TestMenuScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"
#include "chinstrap/src/rendering/VulkanFunctions.h"
#include <vk_mem_alloc.h>

void Game::TestGLScene::OnBegin()
{
    using namespace Chinstrap;
    Application::App::Get().materialManager.MakeMaterial();
    material = Application::App::Get().materialManager.GetMaterial();

    /* This is going to be abstracted once resource loading (of vertices etc.)
     * is implemented, otherwise I'm just going of assumptions which lead to more work in the end
    */
    ChinVulkan::VulkanContext &vulkanContext = Application::App::GetVulkanContext();
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        // StagingBuffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        stagingBufferInfo.size = bufferSize;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo stagingBufferAllocationInfo = {};
        stagingBufferAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingBufferAllocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        stagingBufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(vulkanContext.allocator, &stagingBufferInfo, &stagingBufferAllocationInfo, &stagingBuffer, &stagingAllocation, nullptr);

        void* data;
        vmaMapMemory(vulkanContext.allocator, stagingAllocation, &data);
            memcpy(data, vertices.data(), bufferSize);
        vmaUnmapMemory(vulkanContext.allocator, stagingAllocation);

        // VertexBuffer
        VkBufferCreateInfo vertexBufferCreateInfo = {};
        vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vertexBufferCreateInfo.size = bufferSize;
        vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo vertexBufferAllocationCreateInfo = {};
        vertexBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vertexBufferAllocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        vmaCreateBuffer(vulkanContext.allocator, &vertexBufferCreateInfo, &vertexBufferAllocationCreateInfo, &vertexBuffer, &vertexAllocation, nullptr);

        { // Copy stagingBuffer vertexBuffer to via command Buffer
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = *Application::App::Get().renderContext.aCommandPools.ptrAt(0); // Just to make it work for now
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vulkanContext.virtualGPU, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = bufferSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            // In a system, we should wait for multiple command buffers to copy data concurrently with a fence!
            vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vulkanContext.graphicsQueue);

            vkFreeCommandBuffers(vulkanContext.virtualGPU, *Application::App::Get().renderContext.aCommandPools.ptrAt(0), 1, &commandBuffer);
        }
        // Get rid of staging Buffer
        vmaDestroyBuffer(vulkanContext.allocator, stagingBuffer, nullptr);
        vmaFreeMemory(vulkanContext.allocator, stagingAllocation);
    }
    { // Create Index Buffer

        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        // StagingBuffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        stagingBufferInfo.size = bufferSize;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo stagingBufferAllocationInfo = {};
        stagingBufferAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingBufferAllocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        stagingBufferAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(vulkanContext.allocator, &stagingBufferInfo, &stagingBufferAllocationInfo, &stagingBuffer, &stagingAllocation, nullptr);

        void* data;
        vmaMapMemory(vulkanContext.allocator, stagingAllocation, &data);
            memcpy(data, indices.data(), bufferSize);
        vmaUnmapMemory(vulkanContext.allocator, stagingAllocation);

        // Index Buffer
        VkBufferCreateInfo indexBufferCreateInfo = {};
        indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        indexBufferCreateInfo.size = bufferSize;
        indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo indexBufferAllocationCreateInfo = {};
        indexBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        indexBufferAllocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        vmaCreateBuffer(vulkanContext.allocator, &indexBufferCreateInfo, &indexBufferAllocationCreateInfo, &indexBuffer, &indexAllocation, nullptr);

        { // Copy stagingBuffer vertexBuffer to via command Buffer
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = *Application::App::Get().renderContext.aCommandPools.ptrAt(0); // Just to make it work for now
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vulkanContext.virtualGPU, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = bufferSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, indexBuffer, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            // In a system, we should wait for multiple command buffers to copy data concurrently with a fence!
            vkQueueSubmit(vulkanContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vulkanContext.graphicsQueue);

            vkFreeCommandBuffers(vulkanContext.virtualGPU, *Application::App::Get().renderContext.aCommandPools.ptrAt(0), 1, &commandBuffer);
        }
        // Get rid of staging Buffer
        vmaDestroyBuffer(vulkanContext.allocator, stagingBuffer, nullptr);
        vmaFreeMemory(vulkanContext.allocator, stagingAllocation);
    }
}

void Game::TestGLScene::OnShutdown()
{
    using namespace Chinstrap;
    vmaDestroyBuffer(Application::App::GetVulkanContext().allocator, indexBuffer, indexAllocation);
    vmaDestroyBuffer(Application::App::GetVulkanContext().allocator, vertexBuffer, vertexAllocation);
}

void Game::TestGLScene::OnUpdate(float deltaTime)
{
}

void Game::TestGLScene::OnRender(uint32_t currentFrame)
{
    using namespace Chinstrap;

    ChinVulkan::BeginRendering(standardCmdBufferArray[currentFrame], Application::App::GetVulkanContext(), material->pipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(standardCmdBufferArray[currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(standardCmdBufferArray[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    //vkCmdDraw(standardCmdBufferArray[currentFrame], 3, 1, 0, 0);
    vkCmdDrawIndexed(standardCmdBufferArray[currentFrame], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    ChinVulkan::EndRendering(standardCmdBufferArray[currentFrame], Application::App::GetVulkanContext());
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
