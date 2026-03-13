#pragma once

#include "rendering/Renderer.h"
#include "resourcer/MaterialManager.h"
#include "Window.h"
#include "ops/Logging.h"

namespace Chinstrap {struct Scene;}

namespace Chinstrap::Application
{
    struct App
    {
        // Single object to handle a window and vulkanContext
        Display::Window window;
        UserSettings::GraphicsSettings graphicsSettings;

        Resourcer::MaterialManager materialManager;

        uint32_t framerate = 0;
        bool running;

        App(App const&)  = delete;
        App(App const&&) = delete;
        // Set the expected stackSize to avoid memory fragmentation
        explicit App(uint8_t sceneStackSize);

        int Init();
        void Run(const Display::WindowSpec& windowSpec);
        void Stop();

        // Enforce that the container itself should not be changed in any way after initialization
        static const auto& GetSceneStack()
        {
            return Get().sceneStack;
        }
        static auto& GetVulkanContext()
        {
            return Get().window.vulkanContext;
        }
        static auto& GetWindow()
        {
            return Get().window;
        }
        static auto& GetGraphicsSettings()
        {
            return Get().graphicsSettings;
        }
        static const auto& GetFrameRate()
        {
            return Get().framerate;
        }
        static auto& GetMaterialManager()
        {
            return Get().materialManager;
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

        // Store pointers to scenes which have requested a scene transition
        std::vector<std::unique_ptr<Scene>*> sceneTransitionQueue;

        void Cleanup();

        // Get reference to app singleton
        static App& Get();
    };

}

