#include "Application.h"
#include "Window.h"
#include "Scene.h"
#include "events/Event.h"
#include "ops/Logging.h"
#include "ops/Profiling.h"

#include "GLFW/glfw3.h"

#include <cassert>
#include <memory>

#include "rendering/VulkanFunctions.h"

namespace Chinstrap::Application
{
    namespace
    {
        App *appInstance = nullptr;

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

    int Init(const std::string &appName, Window::FrameSpec &frameSpec, Window::ViewPortSpec &viewportSpec)
    {
        appInstance = new App();
        appInstance->name = appName;

        if (!glfwInit())
        {
            return -1;
        }

        UserSettings::GraphicsSettings settings(UserSettings::VSyncMode::ON, UserSettings::ColorSpaceMode::SRGB);
        appInstance->frame = std::make_unique<Window::Frame>(frameSpec, viewportSpec, settings);

        Window::Create(*appInstance->frame);

        appInstance->frame->EventPassthrough = [](Event& event){ ForwardEvents(event); };

        return 0;
    }

    void Run()
    {
        assert(appInstance);
        appInstance->running = true;

        for (std::unique_ptr<Scene> &scene: appInstance->sceneStack)
            scene->OnBegin();

        double timeAtPreviousFrame = glfwGetTime();
        double timeAtPreviousSecond = glfwGetTime();
        double currentTime = 0.0f;
        int frameCount = 0;

        while (appInstance->running)
        {
            if (Window::ShouldClose(*appInstance->frame))
            {
                Stop();
            }

            Window::Update(*appInstance->frame);
            glfwPollEvents();

            currentTime = glfwGetTime();
            for (std::unique_ptr<Scene> &scene: appInstance->sceneStack)
            {
                // TODO: Implement multithreaded rendering pipeline

                CHIN_PROFILE_TIME(scene->OnUpdate(static_cast<float>((currentTime - timeAtPreviousFrame)*1000)), scene->OnUpdateProfile);

                CHIN_PROFILE_TIME(scene->OnRender(), scene->OnRenderProfile);

                if (scene->CreateQueued != nullptr) // scene has requested change to new scene
                {
                    CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->GetName());
                    scene = std::move(scene->CreateQueued());
                    CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->GetName());
                    scene->OnBegin();
                }
                // DON'T operate on scene in stack after possibly changing the scene
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

        // Cleanup after running
        vkDeviceWaitIdle(appInstance->frame->vulkanContext.virtualGPU);
        Window::Destroy(*appInstance->frame);
        delete appInstance;
    }

    void Stop()
    {
        appInstance->running = false;
    }
}

