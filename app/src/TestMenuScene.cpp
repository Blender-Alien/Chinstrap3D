#include "TestMenuScene.h"

// TODO: Move to Chinstrap:: render call
#include "glad.h"
#include "TestGLScene.h"

#include "GLFW/glfw3.h"
#include "src/InputEvents.h"

void Game::TestMenuScene::OnUpdate()
{
}

void Game::TestMenuScene::OnRender()
{
    // TODO: Move to Chinstrap:: render call
    glClear(GL_COLOR_BUFFER_BIT);
}

bool Game::TestMenuScene::OnKeyPress(Chinstrap::Event &event)
{
    switch (dynamic_cast<Chinstrap::KeyPressedEvent&>(event).keyCode)
    {
        case GLFW_KEY_HOME:
            QueueChangeToScene<TestGLScene>();
            return true;

        case GLFW_KEY_1:
            CHIN_LOG_INFO("We're in the TestMenuScene!!");
            return false;

        default:
            return false;
    }
}

void Game::TestMenuScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::Event &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}