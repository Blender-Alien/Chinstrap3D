#pragma once

#include <string>
#include <vector>
#include <memory>

class GLFWwindow;

namespace Chinstrap
{
    class Scene;

    namespace Application
    {
        struct App
        {
            std::string name;
            bool running;

            GLFWwindow* window;

            std::vector<std::unique_ptr<Scene>> sceneStack;

            // App should be a singleton
            static App& Get();
            App(App const&)  = delete;
            App(App const&&) = delete;
            App();
            ~App();
        };

        int Init(const std::string& appName);
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

