#include "TestGLScene.h"

// TODO: Move to Chinstrap:: render call
#include "glad.h"

#include <iostream>

void Game::TestGLScene::OnEvent(Chinstrap::Event &event)
{
    if (event.GetEventType() == Chinstrap::EventType::WindowClose)
    {
        std::cout << "TestGLScene did something before window closed!!" << std::endl;
        event.handled = true;
    }
}

void Game::TestGLScene::OnUpdate()
{
}

void Game::TestGLScene::OnRender()
{
    // TODO: Move to Chinstrap:: render call
    glClear(GL_COLOR_BUFFER_BIT);
}
