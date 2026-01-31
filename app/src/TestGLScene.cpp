#include "TestGLScene.h"

// TODO: Abstract Render Code
#include <iostream>

#include "../../chinstrap/src/events/InputEvents.h"
#include "chinstrap/src/ops/Logging.h"
#include "chinstrap/src/rendering/Renderer.h"
#include "chinstrap/src/rendering/RendererData.h"

#include "glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TestMenuScene.h"

void Game::TestGLScene::OnBegin()
{

    constexpr float vertices[] = {
        0.5f, 0.5f, 0.0f,   // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f,  // top left
    };
    constexpr unsigned int indices[] = {
        0, 1, 3, // first triangle
        3, 1, 2  // second triangle
    };

    unsigned int vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    unsigned int vertexBufferID;
    glGenBuffers(1, &vertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    Chinstrap::RendererData::Shader shader("../../../chinstrap/res/shaders/BasicVertex.glsl", "../../../chinstrap/res/shaders/BasicFragment.glsl");
    Chinstrap::Renderer::CompileShader(shader);
    Chinstrap::Renderer::BindShader(shader);

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    unsigned int transformLoc = glGetUniformLocation(shader.ID, "transform");
    GLCall(glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans)));

    glBindVertexArray(vertexArrayID);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Game::TestGLScene::OnUpdate(float deltaTime)
{
}

//TODO: Abstract Render Code
void Game::TestGLScene::OnRender()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
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
