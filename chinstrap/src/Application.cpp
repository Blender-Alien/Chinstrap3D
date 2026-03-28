#include "Application.h"
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

    Application::App *pAppInstance = nullptr;

    void ForwardEvents(Event &event)
    {
        CHIN_LOG_INFO(event.ToString());
        for (unsigned int i = Application::App::GetSceneStack().size(); i > 0; i--)
        {
            Application::App::GetSceneStack()[i-1]->OnEvent(event);
            if (event.IsHandled())
                return;
        }
    }
}

namespace Chinstrap::Application
{
    App::App(uint8_t sceneStackSize)
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

int Application::App::Init()
{
    assert(!pAppInstance); // Have we already initialized?
    pAppInstance = this;

    filePathMap.Setup();

    running = false;

    for (auto &scene : sceneTransitionQueue)
    {
        scene = nullptr;
    }

    if (!glfwInit())
    {
        return -1;
    }

    resourceManager.Setup();

    return 0;
}

void Application::App::Run(const Display::WindowSpec &windowSpec)
{
    assert(pAppInstance);
    assert(!running); // Are we already running? Don't call Run() recursively!
    running = true;

    window.Create(windowSpec, graphicsSettings, sceneStack);
    window.EventPassthrough = [](Event& event){ ForwardEvents(event); };

    double timeAtPreviousFrame = glfwGetTime(), timeAtPreviousSecond = glfwGetTime();
    double currentTime = 0.0f;
    uint32_t frameCount = 0;
    uint32_t currentFrame = 0;
    bool skipFrame = false;
    uint8_t sceneIndex = 0;

    for (std::unique_ptr<Scene> &scene: sceneStack)
        scene->OnBegin();

    while (running)
    {
        if (window.ShouldClose()) { Stop(); continue; }

        glfwPollEvents();
        currentTime = glfwGetTime();
        currentFrame = window.vulkanContext.currentFrame;

        { /* Update and Render */
            skipFrame = Renderer::BeginFrame(currentFrame, window.renderContext);

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
                Renderer::SubmitDrawData(currentFrame, window.renderContext);
                Renderer::RenderFrame(currentFrame, window.renderContext);
            }

            sceneIndex = 0;
            for (auto &scene : sceneTransitionQueue)
            {
                if (scene != nullptr) [[unlikely]]
                {
                    vkDeviceWaitIdle(window.vulkanContext.virtualGPU);
                    scene->get()->OnShutdown();

                    CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->get()->GetName());
                    sceneStack.at(sceneIndex) = std::move(scene->get()->CreateQueued(&resourceManager));
                    CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->get()->GetName());

                    Renderer::SetupSceneCmdBuffers(sceneIndex, window.renderContext);
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
            framerate = frameCount;
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
    window.FinishRendering();
    for (auto &scene: sceneStack)
    {
        scene->OnShutdown();
    }
    window.Destroy();
    resourceManager.Cleanup();
    filePathMap.Cleanup();
}