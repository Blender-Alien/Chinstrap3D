#include "TestMenuScene.h"

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"

#include "TestGLScene.h"

#include "GLFW/glfw3.h"

void Game::TestMenuScene::OnBegin()
{
}

void Game::TestMenuScene::OnUpdate(float deltaTime)
{
}

void Game::TestMenuScene::OnRender()
{
}

bool Game::TestMenuScene::OnKeyPress(const Chinstrap::KeyPressedEvent &event)
{
    switch (event.keyCode)
    {
        case GLFW_KEY_HOME:
            if (!event.repeat)
            {
                QueueChangeToScene<TestGLScene>();
                return true;
            }

        case GLFW_KEY_1:
            if (!event.repeat)
            {
                CHIN_LOG_INFO("We're in the TestMenuScene!!");
                return false;
            }

        default:
            return false;
    }
}

void Game::TestMenuScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::KeyPressedEvent &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}
