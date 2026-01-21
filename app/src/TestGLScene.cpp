#include "TestGLScene.h"

// TODO: Move to Chinstrap:: render call
#include "glad.h"

#include <iostream>

#include "GLFW/glfw3.h"
#include "src/InputEvents.h"


void Game::TestGLScene::OnUpdate()
{
}

void Game::TestGLScene::OnRender()
{
    // TODO: Move to Chinstrap:: render call
    glClear(GL_COLOR_BUFFER_BIT);
}

bool OnWindowClose(Chinstrap::Event &event)
{
    std::cout << "TestOpenGLScene-->OnEvent: " << event.ToString() << "\n";
    return true;
}

bool OnKeyPress(Chinstrap::Event &event)
{
    if (dynamic_cast<Chinstrap::KeyPressedEvent&>(event).keyCode == GLFW_KEY_HOME)
    {
        std::cout << "TestOpenGLScene-->OnEvent: " << event.ToString() << "\n";
        return true;
    }
    return false;
}

bool OnKeyRelease(Chinstrap::Event &event)
{
    if(dynamic_cast<Chinstrap::KeyReleasedEvent&>(event).keyCode == GLFW_KEY_HOME)
    {
        std::cout << "TestOpenGLScene-->OnEvent: " << event.ToString() << "\n";
        return true;
    }
    return false;
}

void Game::TestGLScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [](Chinstrap::Event &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyReleasedEvent>(event, [](Chinstrap::Event &dispatchedEvent) { return OnKeyRelease(dispatchedEvent); });
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::WindowClosedEvent>(event, [](Chinstrap::Event &dispatchedEvent) { return OnWindowClose(dispatchedEvent); });
}