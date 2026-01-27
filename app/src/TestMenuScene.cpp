#include <Chinstrap.h>

#include "TestMenuScene.h"

// TODO: Move to Chinstrap:: render call
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "glad.h"

#include "TestGLScene.h"

#include "GLFW/glfw3.h"
#include "src/InputEvents.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_opengl3.h"

Game::TestMenuScene::~TestMenuScene()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Game::TestMenuScene::OnBegin()
{
    //TODO: This is janky, implement real support for imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(Chinstrap::Application::App::Get().frame->window, true);
    ImGui_ImplOpenGL3_Init();
}

void Game::TestMenuScene::OnUpdate()
{
}

void Game::TestMenuScene::OnRender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    // TODO: Move to Chinstrap:: render call
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Game::TestMenuScene::OnKeyPress(Chinstrap::KeyPressedEvent &event)
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
