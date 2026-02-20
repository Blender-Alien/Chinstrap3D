#include "TestGuiScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "../../chinstrap/src/ops/DevInterface.h"

void Game::TestGUIScene::OnBegin()
{
    Chinstrap::DevInterface::Initialize();
}

void Game::TestGUIScene::OnShutdown()
{
    Chinstrap::DevInterface::Shutdown();
    vkDestroyDescriptorPool(Chinstrap::Application::App::GetVulkanContext().virtualGPU, Chinstrap::Application::App::GetVulkanContext().imguiPool, nullptr);
}

void Game::TestGUIScene::OnUpdate(float deltaTime)
{
}

void Game::TestGUIScene::OnRender(uint32_t currentFrame)
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
