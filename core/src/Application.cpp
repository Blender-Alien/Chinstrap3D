#include "Application.h"
#include "Window.h"
#include "Scene.h"

#include "glad.h"
#include "GLFW/glfw3.h"

#include <assert.h>
#include <memory>

namespace Chinstrap
{
    namespace Application
    {

        namespace // access only in this file
        {
            static App* appInstance = nullptr;
        }

        App::App()
        {
            assert(appInstance == nullptr);
        }

        App::~App()
        {
            glfwTerminate();
            appInstance = nullptr;
        }

        App& App::Get()
        {
            assert(appInstance);
            return *appInstance;
        }

        int Init(const std::string& appName, const Window::FrameSpec& spec)
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

                glClear(GL_COLOR_BUFFER_BIT);
                Window::Update(*appInstance->frame);
                glfwPollEvents();
            }

            Stop();
        }

        void Stop()
        {
            appInstance->running = false;
        }
    }
     

}
