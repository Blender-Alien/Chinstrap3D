#include "Application.h"
#include "Window.h"
#include "Scene.h"
#include "events/Event.h"
#include "ops/Logging.h"
#include "ops/Profiling.h"

#include "GLFW/glfw3.h"

#include "rendering/VulkanFunctions.h"

#if __linux__
#include <unistd.h>
#elif _WIN64
#include <windows.h>
#endif

namespace
{
    using namespace Chinstrap;

    Application::App *pAppInstance = nullptr;

    void ForwardEvents(Event &event)
    {
        // Not all event types are worthy of printing out by default
        // TODO: We could control this via config.chin
        switch (event.type)
        {
        case EventType::MouseMoved:
        case EventType::MouseScrolled:
            break;
        default:
            CHIN_LOG_INFO(event.ToString());
        }

        for (unsigned int i = Application::App::GetSceneStack().size(); i > 0; i--)
        {
            Application::App::GetSceneStack()[i-1]->OnEvent(event);
            if (event.handled)
                return;
        }
    }

    void FindProgramPath()
    { // We need to know where our application is in the Operating system filesystem,
        // and we can precalculate the index of our root index in that string.
#if __linux__
        char result[PATH_MAX];
        const auto count = readlink("/proc/self/exe", result, PATH_MAX);
        Application::App::config.programPath = new std::string(result, (count > 0) ? count : 0);
        constexpr char slash = '/';
#elif _WIN64
        char result[MAX_PATH];
        Application::App::config.programPath = new std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
        constexpr char slash = '\\';
#endif
        std::vector<std::size_t> slashPositions;
            { // Find all slashes
            slashPositions.reserve(8); // Guess an initial number to avoid reallocations
            std::size_t position = 0;

            while ((position = Application::App::config.programPath->find(slash, position)) != std::string::npos)
            {
                slashPositions.push_back(position);
                ++position;
            }
            }
        Application::App::config.programPathRootIndex = slashPositions.at(slashPositions.size() - 4) + 1;
    }

    bool ParseEngineConfig()
    {

        auto path = "config.chin"; // We expect this in the project root
        std::ifstream fileStream(Memory::ConvertToOSPath(path).get());
        ENSURE_OR_RETURN_FALSE(fileStream.is_open())

        std::string line;
        while (std::getline(fileStream, line))
        {
            if (auto index = line.find(" #applicationName"); index != SIZE_MAX)
            {
                Application::App::config.appName = line.substr(0, index);
                continue;
            }
            if (auto index = line.find(" #loggingPath"); index != SIZE_MAX)
            {
                Application::App::config.loggingPath = line.substr(0, index);
                continue;
            }
            if (auto index = line.find(" #loggingLevel"); index != SIZE_MAX)
            {
                const std::string value = line.substr(0, index);
                if (value == "warn")
                    spdlog::set_level(spdlog::level::warn);
                else if (value == "err")
                    spdlog::set_level(spdlog::level::err);
                else if (value == "critical")
                    spdlog::set_level(spdlog::level::critical);
                // Automatic default used by spdlog is "info"
                continue;
            }
            if (auto index = line.find(" #loggingFlushInterval"); index != SIZE_MAX)
            {
                Application::App::config.loggingFlushInterval =
                    std::chrono::seconds(std::stoi(line.substr(0, index)));
                continue;
            }
#ifndef CHIN_SHIPPING_BUILD
            if (auto index = line.find(" #loggingFlushLevel"); index != SIZE_MAX)
            {
                spdlog::level::level_enum level;

                const std::string value = line.substr(0, index);
                if (value == "info")
                    level = spdlog::level::info;
                else if (value == "warn")
                    level = spdlog::level::warn;
                else if (value == "err")
                    level = spdlog::level::err;
                else if (value == "critical")
                    level = spdlog::level::critical;
                else
                    continue;

                Application::App::config.loggingFlushLevel = level;
                continue;
            }
#endif
        }

        if (Application::App::config.appName.empty()
            || Application::App::config.loggingPath.empty())
        {
            return false;
        }

        return true;
    }

    void SetupLogging()
    {
        fileLogger = spdlog::basic_logger_mt("chinlog", Memory::ConvertToOSPath(Application::App::config.loggingPath + "chinlog.txt").get());

        /* In a shipping build we don't need constant flushing, and especially not the performance impact of doing so!
         * Instead, we can rely on flush_on which will give us everything as soon as anything interesting happens.
         * That way, we still get a log with warnings, errors and criticals when we didn't exit the program normally,
         * without constantly preemptively writing info logs, while still getting info logs at the end of a normal run!
         */
#ifndef CHIN_SHIPPING_BUILD
        spdlog::flush_every(Application::App::config.loggingFlushInterval);
#endif
        fileLogger->flush_on(Application::App::config.loggingFlushLevel);

        {
            const auto level = spdlog::get_level();
            spdlog::set_level(spdlog::level::info);
            fileLogger.get()->info("Chinstrap App Initialization");
            spdlog::set_level(level);
        }
    }

}

namespace Chinstrap::Application
{
    App::App(uint8_t sceneStackSize)
        : running(false),
          sceneStack(sceneStackSize),
          sceneTransitionQueue(sceneStackSize)
    {
        // If this pointer has a value, we have already initialized an app instance
        assert(!pAppInstance);
    }

    App &App::Get()
    {
        return *pAppInstance;
    }
}

bool Application::App::Init()
{
    assert(!pAppInstance); // Have we already initialized?
    pAppInstance = this;

    running = false;

    for (auto &scene : sceneTransitionQueue)
    {
        scene = nullptr;
    }

    ENSURE_OR_RETURN_FALSE(glfwInit());

    FindProgramPath();
    if (!ParseEngineConfig())
    { // We cannot set up our normal logging when we have failed parsing the config, so we have to do this manually
        // TODO: We can extend this to include info about what went wrong.
        const auto logger =
            spdlog::basic_logger_st("fail_logger", Memory::ConvertToOSPath("fail_log.txt").get());
        logger->info("Attempted to startup chinstrap.");
        logger->critical("Failed to parse config.chin!");
        return false;
    }
    SetupLogging();

    // TODO: Maybe we can serialize the previous usage during runtime and take that as a starting point
    devStrings.Setup(16, 256);
    devStrings.EndSetup();

    resourceManager.Setup();

    return true;
}

void Application::App::Run(const Display::WindowSpec &windowSpec)
{
    assert(pAppInstance);
    assert(!running); // Are we already running? Don't call Run() recursively!
    running = true;

    if (!window.Create(windowSpec, graphicsSettings, sceneStack))
    {
        CHIN_LOG_CRITICAL("Failed to create window!");
        Cleanup();
        return;
    }
    window.eventPassthrough = ForwardEvents;

    double timeAtPreviousFrame = glfwGetTime(), timeAtPreviousSecond = glfwGetTime();
    double currentTime = 0.0f;
    uint32_t frameCount = 0;
    uint32_t currentFrame = 0;
    bool skipFrame = false;
    uint8_t sceneIndex = 0;

    for (std::unique_ptr<Scene> &scene: sceneStack)
        scene->OnBegin();

    while (running)
    {
        if (window.ShouldClose()) { Stop(); continue; }

        glfwPollEvents();
        currentTime = glfwGetTime();
        currentFrame = window.vulkanContext.currentFrame;

        { /* Update and Render */
            skipFrame = Renderer::BeginFrame(currentFrame, window.renderContext);

            if (!skipFrame)
            {
                sceneIndex = 0;
                for (auto &scene : sceneStack)
                {
                    CHIN_PROFILE_TIME(scene->OnUpdate(static_cast<float>((currentTime - timeAtPreviousFrame)*1000)), scene->OnUpdateProfile);
                    if (scene->CreateQueued != nullptr) [[unlikely]] // scene has requested change to new scene
                    {
                        sceneTransitionQueue.at(sceneIndex) = &scene;
                    }

                    CHIN_PROFILE_TIME(scene->OnRender(currentFrame), scene->OnRenderProfile);

                    ++sceneIndex;
                }
                Renderer::SubmitDrawData(currentFrame, window.renderContext);
                Renderer::RenderFrame(currentFrame, window.renderContext);
            }

            sceneIndex = 0;
            for (auto &scene : sceneTransitionQueue)
            {
                if (scene != nullptr) [[unlikely]]
                {
                    vkDeviceWaitIdle(window.vulkanContext.virtualGPU);
                    scene->get()->OnShutdown();

                    CHIN_LOG_INFO("Unreferencing Scene: [{}] ...", scene->get()->GetName());
                    sceneStack.at(sceneIndex) = std::move(scene->get()->CreateQueued(&resourceManager));
                    CHIN_LOG_INFO("... Slotted in Scene: [{}]", scene->get()->GetName());

                    Renderer::SetupSceneCmdBuffers(sceneIndex, window.renderContext);
                    scene->get()->OnBegin();

                    scene = nullptr;
                }
                ++sceneIndex;
            }

        }

        timeAtPreviousFrame = currentTime;
        ++frameCount;
        if (currentTime - timeAtPreviousSecond >= 1.0f)
        {
            framerate = frameCount;
            frameCount = 0;
            timeAtPreviousSecond = currentTime;
        }
    }
    // We're finished running, lets cleanup
    Cleanup();
}

void Application::App::Stop()
{
    running = false;
}

void Application::App::InternString(Memory::DevString& string_arg, const std::string_view& inputString_arg)
{
    auto ret = Get().devStrings.Insert(string_arg, inputString_arg);
    switch (ret)
    {
    case Memory::StringMap::InsertRet::SUCCESS:
        break;
    case Memory::StringMap::InsertRet::COLLISION_OR_DUPLICATE:
        CHIN_LOG_ERROR("App tried to intern a DevString, but there was a collision OR duplicate!");
        break;
    case Memory::StringMap::InsertRet::NO_KEY_CAPACITY:
        CHIN_LOG_ERROR("App tried to intern a DevString, but there was no key capacity!");
        Get().devStrings.GrowBy(10, std::nullopt);
        break;
    case Memory::StringMap::InsertRet::NO_VALUE_CAPACITY:
        CHIN_LOG_ERROR("App tried to intern a DevString, but there was no value capacity!");
        Get().devStrings.GrowBy(10, std::nullopt);
        break;
    case Memory::StringMap::InsertRet::BAD_REQUEST:
        CHIN_LOG_ERROR("App tried to intern a DevString, but it was a bad request!");
        break;
    }
}

std::optional<std::string_view> Application::App::LookupString(const Memory::DevString& key_arg)
{
    return Get().devStrings.Lookup(key_arg);
}

void Application::App::Cleanup()
{
    window.FinishRendering();
    for (auto &scene: sceneStack)
    {
        scene->OnShutdown();
    }
    resourceManager.Cleanup();
    window.Destroy();
    delete config.programPath;
    devStrings.Cleanup();
}
