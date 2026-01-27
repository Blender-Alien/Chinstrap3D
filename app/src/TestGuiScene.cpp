#include "TestGuiScene.h"

#include <Chinstrap.h>
#include "src/InputEvents.h"

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"

Game::TestGUIScene::~TestGUIScene()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Game::TestGUIScene::OnBegin()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(Chinstrap::Application::App::Get().frame->window, true);
    ImGui_ImplOpenGL3_Init();


    float xscale, yscale;
    glfwGetWindowContentScale(Chinstrap::Application::App::Get().frame->window, &xscale, &yscale);


    ImGui::GetStyle().FontScaleDpi = xscale;

    ImFontConfig font_config;
    font_config.RasterizerDensity = xscale;
    ImFont* font = io.Fonts->AddFontFromFileTTF("../../../vendor/fonts/AdwaitaMono/AdwaitaMonoNerdFont-Regular.ttf", 16.0f, &font_config);
    io.Fonts->Build();
}

void Game::TestGUIScene::OnUpdate()
{
}

void Game::TestGUIScene::OnRender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Game::TestGUIScene::OnKeyPress(const Chinstrap::KeyPressedEvent &event)
{
    switch (event.keyCode)
    {
        default:
            return false;
    }
}

void Game::TestGUIScene::OnEvent(Chinstrap::Event &event)
{
    Chinstrap::EventDispatcher::Dispatch<Chinstrap::KeyPressedEvent>(event, [this](Chinstrap::KeyPressedEvent &dispatchedEvent) { return OnKeyPress(dispatchedEvent); });
}
