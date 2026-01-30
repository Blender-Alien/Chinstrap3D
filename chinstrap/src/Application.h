#pragma once

#include <string>
#include <vector>
#include <memory>

#include "events/Event.h"
#include "Window.h"

namespace Chinstrap
{
    struct Scene;
    namespace Window {
        struct Frame;
        struct FrameSpec;
    }

    namespace Application
    {
        struct App
        {
            std::vector<std::unique_ptr<Scene>> sceneStack;
            std::string name;

            std::unique_ptr<Window::Frame> frame;

            int framerate;
            bool running;
            // App should be a singleton
            static App& Get();
            App(App const&)  = delete;
            App(App const&&) = delete;
            App();
            ~App();
        };

        int Init(const std::string& appName, Window::FrameSpec& spec, Window::ViewPortSpec& viewportSpec);
        void Run();
        void Stop();

        template<typename TScene>
        void PushScene()
        {
            App::Get().sceneStack.emplace_back(std::make_unique<TScene>());
        }
    }

    int glfwTest();
}

