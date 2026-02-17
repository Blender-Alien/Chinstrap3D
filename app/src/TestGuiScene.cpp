#include "TestGuiScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "../../chinstrap/src/ops/DevInterface.h"

void Game::TestGUIScene::OnBegin()
{
    Chinstrap::DevInterface::Initialize();

    submitToRender = [this](const uint32_t currentFrame, const VkImageView &imageView, const Chinstrap::ChinVulkan::VulkanContext &vulkanContext)
    {
        Chinstrap::DevInterface::Render([]()
        {
            Chinstrap::DevInterface::ContextInfo(0.7f, 0.0f);
            Chinstrap::DevInterface::PerformanceInfo(0.0f, 0.0f);
        });
        vkResetCommandBuffer(restaurant.commandBuffers[currentFrame], 0);
        Chinstrap::ChinVulkan::RecordImGUICommandBuffer(restaurant.commandBuffers[currentFrame], imageView, restaurant);
    };
}

void Game::TestGUIScene::OnShutdown()
{
    Chinstrap::DevInterface::Shutdown();
}

void Game::TestGUIScene::OnUpdate(float deltaTime)
{
}

void Game::TestGUIScene::OnRender()
{
}

bool Game::TestGUIScene::OnKeyPress(const Chinstrap::KeyPressedEvent &event)
{
    switch (event.keyCode)
    {
        default:
            return false;
    }
}

void Game::TestGUIScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::KeyPressedEvent &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}
