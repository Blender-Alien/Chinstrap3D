#include "Application.h"
#include "UserSettings.h"
#include "Window.h"
#include "Scene.h"
#include "events/Event.h"
#include "ops/Logging.h"
#include "ops/Profiling.h"

#include "GLFW/glfw3.h"

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
        : running(false),
          sceneStack(sceneStackSize),
          sceneTransitionQueue(sceneStackSize)
    {
        // If this pointer has a value, we have already initialized an app instance
        assert(!pAppInstance);
    }

    App &App::Get()
    {
        return *pAppInstance;
    }
}

int Application::App::Init(const std::string &appName, const Window::FrameSpec &frameSpec, const Window::ViewPortSpec &viewportSpec)
{
    assert(!pAppInstance); // Have we already initialized?
    pAppInstance = this;

    running = false;
    name = appName;

    for (auto &scene : sceneTransitionQueue)
    {
        scene = nullptr;
    }

    if (!glfwInit())
    {
        return -1;
    }

    const auto settings = UserSettings::GraphicsSettings(UserSettings::VSyncMode::ON, UserSettings::ColorSpaceMode::SRGB);
    frame.Create(frameSpec, viewportSpec, settings);
    frame.EventPassthrough = [](Event& event){ ForwardEvents(event); };

    return 0;
}

void Application::App::Run()
{
    assert(pAppInstance);
    assert(!running); // Are we already running? Don't call Run() recursively!
    running = true;

    double timeAtPreviousFrame = glfwGetTime(), timeAtPreviousSecond = glfwGetTime();
    double currentTime = 0.0f;
    uint32_t frameCount = 0;
    uint32_t currentFrame = 0;
    bool skipFrame = false;
    uint8_t sceneIndex = 0;

    // Has to be in this order
    renderContext.Create(sceneStack.size());
    for (std::unique_ptr<Scene> &scene: sceneStack)
        scene->OnBegin();

    while (running)
    {
        if (Window::ShouldClose(frame)) { Stop(); continue; }

        glfwPollEvents();
        currentTime = glfwGetTime();
        currentFrame = frame.vulkanContext.currentFrame;

        { /* Update and Render */
            skipFrame = Renderer::BeginFrame(currentFrame, renderContext);

            sceneIndex = 0;
            for (auto &scene : sceneStack)
            {
                CHIN_PROFILE_TIME(scene->OnUpdate(static_cast<float>((currentTime - timeAtPreviousFrame)*1000)), scene->OnUpdateProfile);

                if (scene->CreateQueued != nullptr) [[unlikely]] // scene has requested change to new scene
                {
                    sceneTransitionQueue.at(sceneIndex) = &scene;
                }

                if (!skipFrame) [[likely]]
                {
                    CHIN_PROFILE_TIME(scene->OnRender(currentFrame), scene->OnRenderProfile);
                }
                ++sceneIndex;
            }
            if (!skipFrame) [[likely]]
            {
                Renderer::SubmitDrawData(currentFrame, renderContext);
                Renderer::RenderFrame(currentFrame, renderContext);
            }

            sceneIndex = 0;
            for (auto &scene : sceneTransitionQueue)
            {
                if (scene != nullptr) [[unlikely]]
                {
                    scene->get()->OnShutdown();

                    CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->get()->GetName());
                    sceneStack.at(sceneIndex) = std::move(scene->get()->CreateQueued());
                    CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->get()->GetName());

                    Renderer::SetupSceneCmdBuffers(sceneIndex, renderContext);
                    scene->get()->OnBegin();

                    scene = nullptr;
                }
                ++sceneIndex;
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

void Application::App::Stop()
{
    running = false;
}

void Application::App::Cleanup()
{
    renderContext.Destroy();
    for (std::unique_ptr<Scene> &scene: sceneStack)
    {
        scene->OnShutdown();
    }
    materialManager.Destroy();
    frame.Destroy();
}
