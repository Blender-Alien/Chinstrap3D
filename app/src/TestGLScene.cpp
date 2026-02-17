#include "TestGLScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"

#include "TestMenuScene.h"

void Game::TestGLScene::OnBegin()
{
    submitToRender = [this](const uint32_t currentFrame, const VkImageView &imageView, const Chinstrap::ChinVulkan::VulkanContext &vulkanContext)
    {
        vkResetCommandBuffer(restaurant.commandBuffers[currentFrame], 0);
        Chinstrap::ChinVulkan::ExampleRecordCommandBuffer(restaurant.commandBuffers[currentFrame], imageView, restaurant,
                                               restaurant.materials.front(), vulkanContext);
    };
}

void Game::TestGLScene::OnShutdown()
{
}

void Game::TestGLScene::OnUpdate(float deltaTime)
{
}

void Game::TestGLScene::OnRender()
{
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
