#include "TestGLScene.h"

// TODO: Abstract Render Code
#include "glad.h"
#include "GLFW/glfw3.h"

#include "TestMenuScene.h"
#include "src/InputEvents.h"

void Game::TestGLScene::OnBegin()
{
    //TODO: Abstract Render Code
    constexpr float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f,
    };

    unsigned int vertexBufferID;
    glGenBuffers(1, &vertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

}

void Game::TestGLScene::OnUpdate()
{
}

//TODO: Abstract Render Code
void Game::TestGLScene::OnRender()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

}

bool Game::TestGLScene::OnKeyPress(Chinstrap::Event &event)
{
    switch (dynamic_cast<Chinstrap::KeyPressedEvent&>(event).keyCode)
    {
        case GLFW_KEY_HOME:
            QueueChangeToScene<TestMenuScene>();
            return true;

        case GLFW_KEY_1:
            CHIN_LOG_INFO("We're in the TestGLScene!!");
            return false;

        default:
            return false;
    }
}

void Game::TestGLScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::Event &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}
