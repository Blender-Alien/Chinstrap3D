#pragma once

#include "rendering/Renderer.h"
#include "resourcer/ResourceManager.h"
#include "Window.h"
#include "ops/Logging.h"
#include "UserSettings.h"

namespace Chinstrap {struct Scene;}

namespace Chinstrap::Application
{
    struct App
    {
        // Single object to handle a window and vulkanContext
        Display::Window window;
        UserSettings::GraphicsSettings graphicsSettings;

        Resourcer::ResourceManager resourceManager;

        // These things can be static because they are independent of any program state
        inline static std::string* programPath;
        inline static std::size_t programPathRootIndex;
        inline static std::string appName;

        /**
         * Use for random strings needed at runtime
         */
        Memory::StringMap devStrings;

        inline static uint32_t framerate = 0;
        bool running;

        App(App const&)  = delete;
        App(App const&&) = delete;
        // Set the expected stackSize
        explicit App(uint8_t sceneStackSize);

        int Init();
        void Run(const Display::WindowSpec& windowSpec);
        void Stop();

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

        static void InternString(Memory::DevString& string_arg, const std::string_view& inputString_arg);
        static std::optional<std::string_view> LookupString(const Memory::DevString& key_arg);

        template<typename TScene>
        void PushScene()
        {
            for (auto& scene : sceneStack)
            {
                if (scene == nullptr)
                {
                    scene = std::make_unique<TScene>(&resourceManager);
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

