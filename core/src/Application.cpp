#include "Application.h"
#include "Window.h"
#include "Scene.h"

#include "GLFW/glfw3.h"

#include <cassert>
#include <memory>

namespace Chinstrap::Application
{
    namespace // access only in this file
    {
        App *appInstance = nullptr;
    }

    App::App()
    {
        assert(appInstance == nullptr);
        running = false;
    }

    App::~App()
    {
        glfwTerminate();
        appInstance = nullptr;
    }

    App &App::Get()
    {
        assert(appInstance);
        return *appInstance;
    }

    int Init(const std::string &appName, const Window::FrameSpec &spec)
    {
        appInstance = new App();
        appInstance->name = appName;

        if (!glfwInit())
        {
            return -1;
        }

        appInstance->frame = std::make_shared<Window::Frame>(spec);

        Window::Create(*appInstance->frame);

        return 0;
    }

    void Run()
    {
        appInstance->running = true;

        while (appInstance->running)
        {
            if (Window::ShouldClose(*appInstance->frame))
            {
                appInstance->running = false;
            }

            for (std::unique_ptr<Scene> &scene: appInstance->sceneStack)
            {
                // TODO: Implement multithreaded rendering pipeline
                scene->OnUpdate();
                scene->OnRender();

                Window::Update(*appInstance->frame);
                glfwPollEvents();

                if (scene->queued != nullptr) // scene has requested change to new scene
                {
                    scene = std::move(scene->queued);
                }
                continue; // don't operate on &scene after possibly changing it
            }
        }
        Stop();
    }

    void Stop()
    {
        appInstance->running = false;
    }
}
