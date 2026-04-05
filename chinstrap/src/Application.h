#pragma once

#include "rendering/Renderer.h"
#include "resourcer/ResourceManager.h"
#include "Window.h"
#include "ops/Logging.h"
#include "UserSettings.h"

namespace Chinstrap {struct Scene;}

namespace Chinstrap::Application
{
    /**
     * This gets filled directly at app initialization via parsing "config.chin".
     * If a member is not std::optional, it is guaranteed to have a valid value.
     */
    struct ChinConfig
    {
        std::string* programPath = nullptr;
        std::size_t programPathRootIndex = 0;
        std::string appName;

        std::string loggingPath;
        std::chrono::seconds loggingFlushInterval = std::chrono::seconds(3);
        spdlog::level::level_enum loggingFlushLevel = spdlog::level::warn;
    };

    struct App
    {
        // Config can be static because values are independent of any program state
        inline static ChinConfig config;

        // Single object to handle a window and vulkanContext
        Display::Window window;
        UserSettings::GraphicsSettings graphicsSettings;

        Resourcer::ResourceManager resourceManager;

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

        bool Init();
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

