#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

#include "events/InputEvents.h"
#include "Window.h"

#include "events/WindowEvents.h"
#include "rendering/VulkanFunctions.h"


namespace Chinstrap::Window
{
    ViewPortSpec::ViewPortSpec(float posScaleX, float posScaleY, float sizeScaleX, float sizeScaleY)
        : posScaleX(posScaleX), posScaleY(posScaleY), sizeScaleX(sizeScaleX), sizeScaleY(sizeScaleY)
    {
    }

    FrameSpec::FrameSpec(const std::string &title, int width, int height)
        : title(title), width(width), height(height)
    {
    }

    Frame::Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec,
                 const UserSettings::GraphicsSettings &graphicsSettings)
        : frameSpec(spec), viewPortSpec(viewportSpec), graphicsSettings(graphicsSettings)
    {
    }

    Frame::Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec)
        : frameSpec(spec), viewPortSpec(viewportSpec),
          graphicsSettings()
    {
    }
}

void Chinstrap::Window::Destroy(Frame &frame)
{
    ChinVulkan::Shutdown(frame.vulkanContext);
    if (frame.window)
    {
        glfwDestroyWindow(frame.window);
    }
    frame.window = nullptr;
    glfwTerminate();
}

bool Chinstrap::Window::ShouldClose(const Frame &frame)
{
    return glfwWindowShouldClose(frame.window) != 0;
}

void Chinstrap::Window::Update(const Frame &frame)
{
}

void Chinstrap::Window::Create(Frame &frame)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, 1);

    frame.window = glfwCreateWindow(
        frame.frameSpec.width,
        frame.frameSpec.height,
        frame.frameSpec.title.c_str(),
        nullptr,
        nullptr
    );
    if (!frame.window)
    {
        std::cerr << "GLFW window creation failed!\n";
        assert(false);
    }

    if (!ChinVulkan::Initialize(frame))
    {
        assert(false);
    }

    glfwMakeContextCurrent(frame.window);

    glfwSetWindowUserPointer(frame.window, &frame);

    /* Set all the event callbacks */

    glfwSetFramebufferSizeCallback(frame.window,[](GLFWwindow* handle, int width, int height)
    {
        Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));
        userFrame.vulkanContext.swapChainInadequate = true;

        WindowResizedEvent event(width, height);
        userFrame.EventPassthrough(event);
    });
    glfwSetWindowCloseCallback(frame.window, [](GLFWwindow *handle)
    {
        const Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));

        WindowClosedEvent event;
        userFrame.EventPassthrough(event);
    });
    glfwSetKeyCallback(frame.window, [](GLFWwindow *handle, int key, int scancode, int action, int mods)
    {
        const Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, action == GLFW_REPEAT);
                userFrame.EventPassthrough(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                userFrame.EventPassthrough(event);
                break;
            }
            default:
                assert(false);
        }
    });
}
