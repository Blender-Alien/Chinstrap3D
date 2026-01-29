#include "DevInterface.h"

#include "Application.h"
#include "Window.h"
#include "Roboto/Roboto-Regular.h"

#include "glad.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "spdlog/fmt/bundled/base.h"

void Chinstrap::DevInterface::ContextInfo()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("ContextInfo");

    int x, y;
    glfwGetWindowSize(Application::App::Get().frame->window, &x, &y);
    ImGui::Text("WindowSize: %dx%d", x, y);
    x *= Application::App::Get().frame->frameSpec.dpiScale;
    y *= Application::App::Get().frame->frameSpec.dpiScale;
    ImGui::Text("WindowSize after Scaling: %dx%d", x, y);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    ImGui::Text("glViewportSize: %dx%d", viewport[2], viewport[3]);

    ImGui::End();
}

void Chinstrap::DevInterface::Render(){ Render([](void){}); }
void Chinstrap::DevInterface::Render(std::function<void()> lambda)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    lambda();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Chinstrap::DevInterface::Initialize(float fontSize)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(Application::App::Get().frame->window, true);
    ImGui_ImplOpenGL3_Init();

    float xscale, yscale;
    glfwGetWindowContentScale(Application::App::Get().frame->window, &xscale, &yscale);

    ImGui::GetStyle().FontScaleDpi = xscale;
    ImFontConfig font_config;
    font_config.RasterizerDensity = xscale;
    io.Fonts->AddFontFromMemoryTTF(robotoRegular, sizeof(robotoRegular), fontSize, &font_config);
    io.Fonts->Build();


}
void Chinstrap::DevInterface::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
