#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "Window.h"
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

bool Chinstrap::Display::Window::Create(const WindowSpec &windowSpec_arg, UserSettings::GraphicsSettings &graphicsSettings_arg, const std::vector<std::unique_ptr<Scene>>& sceneStack_arg)
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
        return false;
    }

    ENSURE_OR_RETURN_FALSE(ChinVulkan::Initialize(vulkanContext, glfwWindow, graphicsSettings_arg, windowSpec.title));

    glfwMakeContextCurrent(glfwWindow);

    glfwSetWindowUserPointer(glfwWindow, this);

    /* Set all the event callbacks */

    glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow *handle)
    {
        const Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        auto data = Event::EventDataUnion();

        Event event(EventType::WindowClose, data);
        window.eventPassthrough(event);
    });
    glfwSetWindowSizeCallback(glfwWindow,[](GLFWwindow* handle, int width, int height)
    {
        Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));
        window.vulkanContext.swapChainInadequate = true;

        auto data = Event::EventDataUnion();
        data.WindowResized.width = width;
        data.WindowResized.height = height;

        Event event(EventType::WindowResize, data);
        window.eventPassthrough(event);
    });
    glfwSetKeyCallback(glfwWindow, [](GLFWwindow *handle, int key, int scancode, int action, int mods)
    {
        const Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                auto data = Event::EventDataUnion();
                data.KeyPressed.keyCode = key;
                data.KeyPressed.repeat = GLFW_REPEAT;

                Event event(EventType::KeyPressed, data);
                window.eventPassthrough(event);
                break;
            }
            case GLFW_RELEASE:
            {
                auto data = Event::EventDataUnion();
                data.KeyReleased.keyCode = key;
                Event event(EventType::KeyReleased, data);
                window.eventPassthrough(event);
                break;
            }
            default: ;
        }
    });
    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow* handle, int button, int action, int mods)
    {
        const Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        switch (action)
        {
        case GLFW_PRESS:
            {
                auto data = Event::EventDataUnion();
                data.MouseButtonPressed.mouseButton = button;

                Event event(EventType::MouseButtonPressed, data);
                window.eventPassthrough(event);
                break;
            }
        case GLFW_RELEASE:
            {
                auto data = Event::EventDataUnion();
                data.MouseButtonReleased.mouseButton = button;

                Event event(EventType::MouseButtonReleased, data);
                window.eventPassthrough(event);
                break;
            }
        default: ;
        }
    });
    glfwSetCursorPosCallback(glfwWindow,[](GLFWwindow* handle, double moveX, double moveY)
    {
        Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        auto data = Event::EventDataUnion();
        data.MouseMoved.mouseX = moveX;
        data.MouseMoved.mouseY = moveY;

        Event event(EventType::MouseMoved, data);
        window.eventPassthrough(event);
    });
    glfwSetScrollCallback(glfwWindow, [](GLFWwindow* handle, double offsetX, double offsetY)
    {
        Window &window = *static_cast<Window *>(glfwGetWindowUserPointer(handle));

        auto data = Event::EventDataUnion();
        data.MouseScrolled.mouseOffsetY = offsetY;

        Event event(EventType::MouseScrolled, data);
        window.eventPassthrough(event);
    });

    ENSURE_OR_RETURN_FALSE(renderContext.Create(&vulkanContext, this, &graphicsSettings_arg, &sceneStack_arg));
    return true;
}

void Chinstrap::Display::Window::FinishRendering() const
{
     vkDeviceWaitIdle(vulkanContext.virtualGPU);
}
