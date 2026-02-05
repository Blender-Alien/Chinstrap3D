#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>
#include <vector>
#include <memory>

#include "Window.h"

namespace Chinstrap {struct Scene;}
namespace Chinstrap::Window {struct Frame; struct FrameSpec;}

namespace Chinstrap::Application
{
    struct App
    {
        std::string name;
        std::vector<std::unique_ptr<Scene>> sceneStack;

        std::unique_ptr<Window::Frame> frame;

        int framerate = 0;
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

