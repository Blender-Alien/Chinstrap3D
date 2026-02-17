#include "Application.h"
#include "UserSettings.h"
#include "Window.h"
#include "Scene.h"
#include "events/Event.h"
#include "ops/Logging.h"
#include "ops/Profiling.h"

#include "GLFW/glfw3.h"

#include <cassert>
#include <memory>

#include "rendering/Renderer.h"
#include "rendering/VulkanFunctions.h"

namespace
{
    using namespace Chinstrap;
    Application::App *appInstance = nullptr;

    void ForwardEvents(Event &event)
    {
        CHIN_LOG_INFO(event.ToString());
        for (unsigned int i = appInstance->sceneStack.size(); i > 0; i--)
        {
            appInstance->sceneStack[i-1]->OnEvent(event);
            if (event.IsHandled())
                return;
        }
    }
}

namespace Chinstrap::Application
{
    App::App()
    {
        assert(appInstance == nullptr);
        running = false;
    }

    App &App::Get()
    {
        assert(appInstance);
        return *appInstance;
    }
}

int Chinstrap::Application::Init(const std::string &appName, Window::FrameSpec &frameSpec, Window::ViewPortSpec &viewportSpec)
{
    appInstance = new App();
    appInstance->name = appName;

    if (!glfwInit())
    {
        return -1;
    }

    UserSettings::GraphicsSettings settings = UserSettings::GraphicsSettings(UserSettings::VSyncMode::ON, UserSettings::ColorSpaceMode::SRGB);
    appInstance->frame = std::make_unique<Window::Frame>(frameSpec, viewportSpec, settings);
    Window::Create(*appInstance->frame);

    appInstance->frame->EventPassthrough = [](Event& event){ ForwardEvents(event); };

    return 0;
}

void Chinstrap::Application::Run()
{
    assert(appInstance);
    appInstance->running = true;

    for (std::unique_ptr<Scene> &scene: appInstance->sceneStack)
        scene->OnBegin();

    double timeAtPreviousFrame = glfwGetTime(), timeAtPreviousSecond = glfwGetTime();
    double currentTime = 0.0f;
    int frameCount = 0;

    bool render = true;
    uint32_t currentFrame = 0;
    Renderer::Setup();

    while (appInstance->running)
    {
        if (Window::ShouldClose(*appInstance->frame))
        {
            Stop();
        }

        glfwPollEvents();

        currentTime = glfwGetTime();

        currentFrame = appInstance->frame->vulkanContext.currentFrame;
        render = Renderer::BeginFrame(currentFrame);
        for (auto &scene : appInstance->sceneStack)
        {
            CHIN_PROFILE_TIME(scene->OnUpdate(static_cast<float>((currentTime - timeAtPreviousFrame)*1000)), scene->OnUpdateProfile);
            if (render) [[likely]]
            {
                CHIN_PROFILE_TIME(scene->OnRender(), scene->OnRenderProfile);
            }

            if (scene->CreateQueued != nullptr) // scene has requested change to new scene
            {
                CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->GetName());
                scene = std::move(scene->CreateQueued());
                CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->GetName());
                scene->OnBegin();
            }
            // DON'T operate on scene in stack after possibly "thisScene"
        }
        if (render) [[likely]]
        {
            Renderer::SubmitDrawData(currentFrame);
            Renderer::RenderFrame(currentFrame);
        }
        timeAtPreviousFrame = currentTime;

        ++frameCount;
        if (currentTime - timeAtPreviousSecond >= 1.0f)
        {
            Application::App::Get().framerate = frameCount;
            frameCount = 0;
            timeAtPreviousSecond = currentTime;
        }
    }
    /* Cleanup after running */

    for (std::unique_ptr<Scene> &scene: appInstance->sceneStack)
    {
        scene->OnShutdown();
    }

    Renderer::Shutdown(appInstance->frame->vulkanContext);
    Window::Destroy(*appInstance->frame);

    delete appInstance;
}

void Chinstrap::Application::Stop()
{
    appInstance->running = false;
}

