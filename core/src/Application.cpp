#include "Application.h"
#include "Window.h"
#include "Scene.h"
#include "Event.h"
#include "Logging.h"

#include "GLFW/glfw3.h"

#include <cassert>
#include <memory>

namespace Chinstrap
{
    namespace Application
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
                    if (event.handled)
                        return;
                }
            }
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

        int Init(const std::string &appName, Window::FrameSpec &spec)
        {
            appInstance = new App();
            appInstance->name = appName;

            if (!glfwInit())
            {
                return -1;
            }

            appInstance->frame = std::make_unique<Window::Frame>(spec);

            Window::Create(*appInstance->frame);

            appInstance->frame->EventPassthrough = [](Event& event){ ForwardEvents(event); };

            return 0;
        }

        void Run()
        {
            assert(appInstance);
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
                    // DON'T operate on scene in stack after possibly changing the scene
                }
            }
        }

        void Stop()
        {
            appInstance->running = false;
        }


    }
}
