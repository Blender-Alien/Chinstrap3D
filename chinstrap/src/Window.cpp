#include "glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

#include "events/InputEvents.h"
#include "Window.h"

#include "ops/Logging.h"
#include "events/WindowEvents.h"
#include "rendering/Renderer.h"


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
        if (window)
        {
            glfwDestroyWindow(window);
        }
        window = nullptr;
    }

    void SetGLFWCallbacks(Frame &frame);

    void Create(Frame &frame)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

        float xscale, yscale;
        glfwGetWindowContentScale(frame.window, &xscale, &yscale);
        CHIN_LOG_INFO("Window scale: {0} by {1}", xscale, yscale);
        frame.frameSpec.dpiScale = xscale;
// Currently on Wayland with glfw-3.4 windowSize doesn't reflect the actual size in screen pixels (29.01.2026)
#if defined(__linux__)
        glfwSetWindowSize(frame.window, frame.frameSpec.width / xscale, frame.frameSpec.height / yscale);
#elif defined(_WIN64)
        glfwSetWindowSize(frame.window, frame.frameSpec.width, frame.frameSpec.height);
#endif


        glfwMakeContextCurrent(frame.window);

        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
        {
            std::cerr << "Glad could not load!\n";
            assert(false);
        }

        glfwSetWindowSizeLimits(frame.window, 720, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
        //TODO: Proper aspect ratio handling
        glfwSetWindowAspectRatio(frame.window, 16, 9);

        GLCall(glViewport(
            static_cast<int>(frame.viewPortSpec.posScaleX * frame.frameSpec.width),
            static_cast<int>(frame.viewPortSpec.posScaleY * frame.frameSpec.height),
            static_cast<int>(frame.viewPortSpec.sizeScaleX * frame.frameSpec.width),
            static_cast<int>(frame.viewPortSpec.sizeScaleY * frame.frameSpec.height)
            ));

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
        glfwSetWindowSizeCallback(frame.window, [](GLFWwindow *handle, int width, int height)
        {
            Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));

// Currently on Wayland with glfw-3.4 windowSize doesn't reflect the actual size in screen pixels (29.01.2026)
#if defined(__linux__)
            GLCall(glViewport(
                static_cast<int>(userFrame.viewPortSpec.posScaleX * width * userFrame.frameSpec.dpiScale),
                static_cast<int>(userFrame.viewPortSpec.posScaleY * height * userFrame.frameSpec.dpiScale),
                static_cast<int>(userFrame.viewPortSpec.sizeScaleX * width * userFrame.frameSpec.dpiScale),
                static_cast<int>(userFrame.viewPortSpec.sizeScaleY * height * userFrame.frameSpec.dpiScale)
            ));
#elif defined(_WIN64)
            GLCall(glViewport(
                static_cast<int>(userFrame.viewPortSpec.posScaleX * width),
                static_cast<int>(userFrame.viewPortSpec.posScaleY * height),
                static_cast<int>(userFrame.viewPortSpec.sizeScaleX * width),
                static_cast<int>(userFrame.viewPortSpec.sizeScaleY * height)
            ));
#endif

            glfwSetWindowSize(userFrame.window, width, height);

            WindowResizedEvent event(width, height);
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

