#pragma once

#include <string>
#include <vector>
#include <memory>

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
            std::string name;
            bool running;

            std::shared_ptr<Window::Frame> frame;

            std::vector<std::unique_ptr<Scene>> sceneStack;

            // App should be a singleton
            static App& Get();
            App(App const&)  = delete;
            App(App const&&) = delete;
            App();
            ~App();
        };

        int Init(const std::string& appName, const Window::FrameSpec& spec);
        void Run();
        void Stop();

        template<typename TScene>
        void PushScene()
        {
            App::Get().sceneStack.push_back(std::make_unique<TScene>());
        }
    }

    int glfwTest();
}

