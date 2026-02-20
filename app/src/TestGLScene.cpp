#include "TestGLScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"
#include "chinstrap/src/rendering/VulkanFunctions.h"

#include "TestMenuScene.h"

Game::TestGLScene::TestGLScene()
    : material(Chinstrap::Application::App::GetVulkanContext(),
        readFile("../../../chinstrap/res/shaders/BasicVertex.spv"),
        readFile("../../../chinstrap/res/shaders/BasicFragment.spv"))
{
}

void Game::TestGLScene::OnBegin()
{
}

void Game::TestGLScene::OnShutdown()
{
    material.Cleanup();
}

void Game::TestGLScene::OnUpdate(float deltaTime)
{
}

void Game::TestGLScene::OnRender(uint32_t currentFrame)
{
    using namespace Chinstrap;

    ChinVulkan::ExampleRecordCommandBuffer(standardCmdBufferArray[currentFrame], Application::App::GetVulkanContext(), material.pipeline);
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
