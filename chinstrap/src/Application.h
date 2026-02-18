#pragma once

#include "Window.h"
#include "ops/Logging.h"

namespace Chinstrap {struct Scene;}
namespace Chinstrap::Window {struct Frame; struct FrameSpec;}

namespace Chinstrap::Application
{
    struct App
    {
        std::string name;


        // Single object to handle a window and vulkanContext
        Window::Frame frame;

        uint32_t framerate = 0;
        bool running;

        // Get reference to app singleton
        static App& Get();

        App(App const&)  = delete;
        App(App const&&) = delete;
        // Set the expected stackSize to avoid memory fragmentation
        explicit App(const uint8_t sceneStackSize);

        int Init(const std::string& appName, const Window::FrameSpec& frameSpec, const Window::ViewPortSpec& viewportSpec);
        void Run();
        void Stop();

        // Enforce that the container itself should not be changed in any way after initialization
        const auto& GetSceneStack()
        {
            return sceneStack;
        }

        template<typename TScene>
        void PushScene()
        {
            for (auto& scene : sceneStack)
            {
                if (scene == nullptr)
                {
                    scene = std::make_unique<TScene>();
                    return;
                }
            }
            assert(false);
            CHIN_LOG_ERROR("No available slot in sceneStack, increase initial size when calling constructor on App!");
        }

    private:
        std::vector<std::unique_ptr<Scene>> sceneStack;
        void Cleanup();
    };

}

