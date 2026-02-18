#include "Application.h"
#include "UserSettings.h"
#include "Window.h"
#include "Scene.h"
#include "events/Event.h"
#include "ops/Logging.h"
#include "ops/Profiling.h"

#include "GLFW/glfw3.h"

#include "rendering/Renderer.h"
#include "rendering/VulkanFunctions.h"

namespace
{
    using namespace Chinstrap;

    // we give out this pointer to the actual stack allocated singleton
    Application::App *pAppInstance = nullptr;

    void ForwardEvents(Event &event)
    {
        CHIN_LOG_INFO(event.ToString());
        for (unsigned int i = pAppInstance->GetSceneStack().size(); i > 0; i--)
        {
            pAppInstance->GetSceneStack()[i-1]->OnEvent(event);
            if (event.IsHandled())
                return;
        }
    }
}

namespace Chinstrap::Application
{
    App::App(const uint8_t sceneStackSize)
        : sceneStack(sceneStackSize), frame(), running(false)
    {
        // If this pointer has a value, we have already initialized an app instance
        assert(!pAppInstance);
    }

    App &App::Get()
    {
        return *pAppInstance;
    }
}

int Chinstrap::Application::App::Init(const std::string &appName, const Window::FrameSpec &frameSpec, const Window::ViewPortSpec &viewportSpec)
{
    assert(!pAppInstance); // Have we already initialized?
    pAppInstance = this;

    running = false;
    name = appName;

    if (!glfwInit())
    {
        return -1;
    }

    const auto settings = UserSettings::GraphicsSettings(UserSettings::VSyncMode::ON, UserSettings::ColorSpaceMode::SRGB);
    frame.Create(frameSpec, viewportSpec, settings);
    frame.EventPassthrough = [](Event& event){ ForwardEvents(event); };

    return 0;
}

void Chinstrap::Application::App::Run()
{
    assert(pAppInstance);
    assert(!running); // Are we already running? Don't call Run() recursively!
    running = true;

    double timeAtPreviousFrame = glfwGetTime(), timeAtPreviousSecond = glfwGetTime();
    double currentTime = 0.0f;
    uint32_t frameCount = 0;
    uint32_t currentFrame = 0;
    bool skipFrame = false;

    for (std::unique_ptr<Scene> &scene: sceneStack)
        scene->OnBegin();

    Renderer::Setup();
    while (running)
    {
        if (Window::ShouldClose(frame)) { Stop(); continue; }

        glfwPollEvents();
        currentTime = glfwGetTime();
        currentFrame = frame.vulkanContext.currentFrame;

        { /* Update and Render */
            skipFrame = Renderer::BeginFrame(currentFrame);
            for (auto &scene : sceneStack)
            {
                CHIN_PROFILE_TIME(scene->OnUpdate(static_cast<float>((currentTime - timeAtPreviousFrame)*1000)), scene->OnUpdateProfile);
                if (!skipFrame) [[likely]]
                {
                    CHIN_PROFILE_TIME(scene->OnRender(), scene->OnRenderProfile);
                }

                if (scene->CreateQueued != nullptr) [[unlikely]] // scene has requested change to new scene
                {
                    CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->GetName());
                    scene = std::move(scene->CreateQueued());
                    CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->GetName());

                    scene->restaurant.Initialize(&frame.vulkanContext);
                    scene->OnBegin();
                }
                // DON'T operate on scene in stack after possibly switching it out
            }
            if (!skipFrame) [[likely]]
            {
                Renderer::SubmitDrawData(currentFrame);
                Renderer::RenderFrame(currentFrame);
            }
        }

        timeAtPreviousFrame = currentTime;
        ++frameCount;
        if (currentTime - timeAtPreviousSecond >= 1.0f)
        {
            pAppInstance->framerate = frameCount;
            frameCount = 0;
            timeAtPreviousSecond = currentTime;
        }
    }
    // We're finished running, lets cleanup
    Cleanup();
}

void Chinstrap::Application::App::Stop()
{
    running = false;
}

void Application::App::Cleanup()
{
    for (std::unique_ptr<Scene> &scene: sceneStack)
    {
        scene->OnShutdown();
    }
    Renderer::Shutdown(frame.vulkanContext);
    frame.Destroy();
}
