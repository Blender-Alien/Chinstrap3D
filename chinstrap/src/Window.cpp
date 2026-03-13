#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "events/InputEvents.h"
#include "Window.h"
#include "events/WindowEvents.h"
#include "rendering/VulkanFunctions.h"

Chinstrap::Display::Window::Window()
{
}

void Chinstrap::Display::Window::Destroy()
{
    renderContext.Destroy();
    ChinVulkan::Shutdown(vulkanContext);
    if (glfwWindow)
    {
        glfwDestroyWindow(glfwWindow);
    }
    glfwWindow = nullptr;
    glfwTerminate();
}

bool Chinstrap::Display::Window::ShouldClose() const
{
    return glfwWindowShouldClose(glfwWindow) != 0;
}

void Chinstrap::Display::Window::Create(const WindowSpec &windowSpec_arg, UserSettings::GraphicsSettings &graphicsSettings_arg, const std::vector<std::unique_ptr<Scene>>& sceneStack_arg)
{
    windowSpec = windowSpec_arg;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, 1);

    glfwWindow = glfwCreateWindow(
        windowSpec.width,
        windowSpec.height,
        windowSpec.title,
        nullptr,
        nullptr
    );
    if (!glfwWindow)
    {
        CHIN_LOG_CRITICAL("GLFW window creation failed!");
        assert(false);
    }

    if (!ChinVulkan::Initialize(vulkanContext, glfwWindow, graphicsSettings_arg, windowSpec.title))
    {
        CHIN_LOG_CRITICAL("Vulkan initialization failed!");
        assert(false);
    }

    glfwMakeContextCurrent(glfwWindow);

    glfwSetWindowUserPointer(glfwWindow, this);

    /* Set all the event callbacks */

    glfwSetFramebufferSizeCallback(glfwWindow,[](GLFWwindow* handle, int width, int height)
    {
        Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));
        window.vulkanContext.swapChainInadequate = true;

        WindowResizedEvent event(width, height);
        window.EventPassthrough(event);
    });
    glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow *handle)
    {
        const Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        WindowClosedEvent event;
        window.EventPassthrough(event);
    });
    glfwSetKeyCallback(glfwWindow, [](GLFWwindow *handle, int key, int scancode, int action, int mods)
    {
        const Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, action == GLFW_REPEAT);
                window.EventPassthrough(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                window.EventPassthrough(event);
                break;
            }
            default:
                assert(false);
        }
    });

    renderContext.Create(&vulkanContext, this, &graphicsSettings_arg, &sceneStack_arg);
}

void Chinstrap::Display::Window::FinishRendering() const
{
 vkDeviceWaitIdle(vulkanContext.virtualGPU);
}
