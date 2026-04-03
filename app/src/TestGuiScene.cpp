#include "TestGuiScene.h"

#include "../../chinstrap/src/ops/DevInterface.h"

#include "chinstrap/src/rendering/VulkanFunctions.h"

void Game::TestGUIScene::OnBegin()
{
    Chinstrap::DevInterface::Initialize();
}

void Game::TestGUIScene::OnShutdown()
{
    Chinstrap::DevInterface::Shutdown();
}

void Game::TestGUIScene::OnUpdate(float deltaTime)
{
}

void Game::TestGUIScene::OnRender(uint32_t currentFrame)
{
    Chinstrap::DevInterface::Render([]()
    {
        Chinstrap::DevInterface::ContextInfo(0.7f, 0.0f);
        Chinstrap::DevInterface::PerformanceInfo(0.0f, 0.0f);
    });

    using namespace Chinstrap;
    ChinVulkan::ExampleRecordDevInterfaceCommandBuffer(standardCmdBufferArray[currentFrame], Application::App::GetVulkanContext());
}

void Game::TestGUIScene::OnEvent(Chinstrap::Event &event)
{
}