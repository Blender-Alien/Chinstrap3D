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

bool OnWindowClose()
{
    std::cout << "TestGLScene did something before window closed!!" << std::endl;
    return true;
}

bool OnKeyPress(Chinstrap::Event &event)
{
    if (dynamic_cast<Chinstrap::KeyPressedEvent&>(event).keyCode == GLFW_KEY_HOME)
    {
        std::cout << "TestGLScene::OnKeyPressed()" << std::endl;
        return true;
    }
    return false;
}

bool OnKeyRelease(Chinstrap::Event &event)
{
    if(dynamic_cast<Chinstrap::KeyReleasedEvent&>(event).keyCode == GLFW_KEY_HOME)
    {
        std::cout << "TestGLScene::OnKeyReleased()" << std::endl;
        return true;
    }
    return false;
}

void Game::TestGLScene::OnEvent(Chinstrap::Event &event)
{
    //TODO: Elegant event dispatcher
    using namespace Chinstrap;
    switch (event.GetEventType())
    {
        case EventType::WindowClose:
            event.handled = OnWindowClose();
            break;
        case EventType::WindowResize:
            break;
        case EventType::KeyPressed:
            event.handled = OnKeyPress(event);
            break;
        case EventType::KeyReleased:
            event.handled = OnKeyRelease(event);
            break;
        default:
            return;
    }
}