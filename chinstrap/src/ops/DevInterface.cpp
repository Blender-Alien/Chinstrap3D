#include "DevInterface.h"

#include "../Application.h"
#include "../Window.h"
#include "Roboto/Roboto-Regular.h"

#include "glad.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "../Scene.h"

void Chinstrap::DevInterface::ContextInfo(float posScaleX, float posScaleY)
{
// This is wasted performance for release builds
#ifdef CHIN_DEBUG
    int x, y;
    glfwGetWindowSize(Application::App::Get().frame->window, &x, &y);

    ImGui::SetNextWindowPos(ImVec2(x * posScaleX, y * posScaleY));
    ImGui::Begin("ContextInfo");

    ImGui::Text("WindowSize: %dx%d", x, y);

    // Currently on Wayland with glfw-3.4 windowSize doesn't reflect the actual size in screen pixels (29.01.2026)
#if defined(__linux__)
    x *= Application::App::Get().frame->frameSpec.dpiScale;
    y *= Application::App::Get().frame->frameSpec.dpiScale;
    ImGui::Text("WindowSize after Scaling: %dx%d", x, y);
#endif

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    ImGui::Text("glViewportSize: %dx%d", viewport[2], viewport[3]);


    ImGui::End();
#endif
}

// For simple fps counter use different method
void Chinstrap::DevInterface::PerformanceInfo(float posScaleX, float posScaleY)
{
#ifdef CHIN_DEBUG
    int x, y;
    glfwGetWindowSize(Application::App::Get().frame->window, &x, &y);
    ImGui::SetNextWindowPos(ImVec2(x * posScaleX, y * posScaleY));

    ImGui::Begin("Performance");
    ImGui::Text("%d FPS", Application::App::Get().framerate);

    ImGui::Checkbox("VSync", &Application::App::Get().frame->frameSpec.vSync);
    glfwSwapInterval(Application::App::Get().frame->frameSpec.vSync? 1 : 0);

    for (std::unique_ptr<Scene> &scene: Application::App::Get().sceneStack)
    {
        ImGui::Text("(%fms): OnUpdate() <- [%s]", scene->OnUpdateProfile, scene->GetName().c_str());
        ImGui::Text("(%fms): OnRender() <- [%s]", scene->OnRenderProfile, scene->GetName().c_str());
    }

    ImGui::End();
#endif
}

void Chinstrap::DevInterface::Render(){ Render([](){}); }
void Chinstrap::DevInterface::Render(void(*lambda)())
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

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
