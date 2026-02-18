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
    ViewPortSpec::ViewPortSpec() {}

    void ViewPortSpec::Create(float posScaleX, float posScaleY, float sizeScaleX, float sizeScaleY)
    {
        this->posScaleX = posScaleX;
        this->posScaleY = posScaleY;
        this->sizeScaleX = sizeScaleX;
        this->sizeScaleY = sizeScaleY;
    }

    FrameSpec::FrameSpec() {}

    void FrameSpec::Create(const std::string &title, int width, int height)
    {
        this->title = title;
        this->width = width;
        this->height = height;
    }

    Frame::Frame() {}

    void Frame::Destroy()
    {
        ChinVulkan::Shutdown(vulkanContext);
        if (window)
        {
            glfwDestroyWindow(window);
        }
        window = nullptr;
        glfwTerminate();
    }

}

bool Chinstrap::Window::ShouldClose(const Frame &frame)
{
    return glfwWindowShouldClose(frame.window) != 0;
}

void Chinstrap::Window::Update(const Frame &frame)
{
}

void Chinstrap::Window::Frame::Create(const FrameSpec &FrameSpec, const ViewPortSpec &ViewportSpec,
                 const UserSettings::GraphicsSettings &GraphicsSettings)
{
    this->frameSpec = FrameSpec;
    this->viewPortSpec = ViewportSpec;
    this->graphicsSettings = GraphicsSettings;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, 1);

    window = glfwCreateWindow(
        frameSpec.width,
        frameSpec.height,
        frameSpec.title.c_str(),
        nullptr,
        nullptr
    );
    if (!window)
    {
        std::cerr << "GLFW window creation failed!\n";
        assert(false);
    }

    if (!ChinVulkan::Initialize(*this))
    {
        assert(false);
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);

    /* Set all the event callbacks */

    glfwSetFramebufferSizeCallback(window,[](GLFWwindow* handle, int width, int height)
    {
        Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));
        userFrame.vulkanContext.swapChainInadequate = true;

        WindowResizedEvent event(width, height);
        userFrame.EventPassthrough(event);
    });
    glfwSetWindowCloseCallback(window, [](GLFWwindow *handle)
    {
        const Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));

        WindowClosedEvent event;
        userFrame.EventPassthrough(event);
    });
    glfwSetKeyCallback(window, [](GLFWwindow *handle, int key, int scancode, int action, int mods)
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
