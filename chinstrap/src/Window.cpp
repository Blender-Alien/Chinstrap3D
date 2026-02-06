#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

#include "events/InputEvents.h"
#include "Window.h"

#include "ops/Logging.h"
#include "events/WindowEvents.h"
#include "rendering/VulkanFunctions.h"


namespace Chinstrap::Window
{
    ViewPortSpec::ViewPortSpec(float posScaleX, float posScaleY, float sizeScaleX, float sizeScaleY)
        : posScaleX(posScaleX), posScaleY(posScaleY), sizeScaleX(sizeScaleX), sizeScaleY(sizeScaleY)
    {
    }

    FrameSpec::FrameSpec(const std::string &title, int width, int height, bool isResizable,
                         bool vSync)
        : title(title), width(width), height(height), isResizable(isResizable), vSync(vSync)
    {
    }

    Frame::Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec)
        : frameSpec(spec), viewPortSpec(viewportSpec)
    {
    }

    Frame::~Frame()
    {
        ChinVulkan::Shutdown(this->vulkanSetupData);
        if (window)
        {
            glfwDestroyWindow(window);
        }
        window = nullptr;
    }

    void SetGLFWCallbacks(Frame &frame);

    void Create(Frame &frame)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

        ChinVulkan::Init(frame.vulkanSetupData, frame.frameSpec.title);
        ChinVulkan::PickPhysicalGPU(frame.vulkanSetupData);
        ChinVulkan::CreateVirtualGPU(frame.vulkanSetupData);

        glfwMakeContextCurrent(frame.window);

        //TODO: Proper aspect ratio handling
        glfwSetWindowAspectRatio(frame.window, 16, 9);

        glfwSwapInterval(frame.frameSpec.vSync ? 1 : 0);

        SetGLFWCallbacks(frame);
    }

    bool ShouldClose(const Frame &frame)
    {
        return glfwWindowShouldClose(frame.window) != 0;
    }

    void Update(const Frame &frame)
    {
        glfwSwapBuffers(frame.window);
    }

    void SetGLFWCallbacks(Frame &frame)
    {
        glfwSetWindowUserPointer(frame.window, &frame);

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
}

