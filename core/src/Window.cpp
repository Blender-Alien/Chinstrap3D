#include "Window.h"
#include "glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

#include "InputEvents.h"
#include "WindowEvents.h"

namespace Chinstrap
{
    namespace Window
    {
        FrameSpec::FrameSpec(const std::string &title, const int &width, const int &height, bool isResizable,
                             bool vSync)
            : Title(title), Width(width), Height(height), IsResizable(isResizable), VSync(vSync)
        {
        }

        Frame::Frame(const FrameSpec &spec)
            : frameSpec(spec)
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

        void SetGLFWCallbacks(Frame& frame);
        void Create(Frame &frame)
        {
            frame.window = glfwCreateWindow(
                frame.frameSpec.Width,
                frame.frameSpec.Height,
                frame.frameSpec.Title.c_str(),
                nullptr,
                nullptr
            );
            if (!frame.window)
            {
                std::cerr << "GLFW window creation failed!\n";
                assert(false);
            }

            glfwMakeContextCurrent(frame.window);

            if (gladLoadGL() == 0)
            {
                std::cerr << "Glad could not load!\n";
                assert(false);
            }

            glfwSwapInterval(frame.frameSpec.VSync ? 1 : 0);

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

        void SetGLFWCallbacks(Frame& frame)
        {
            glfwSetWindowUserPointer(frame.window, &frame);

            glfwSetWindowCloseCallback(frame.window, [](GLFWwindow* handle)
            {
                Frame& frame = *static_cast<Frame*>(glfwGetWindowUserPointer(handle));

                WindowClosedEvent event;
                frame.EventPassthrough(event);
            });

            glfwSetWindowSizeCallback(frame.window, [](GLFWwindow* handle, int width, int height)
            {
                Frame& frame = *static_cast<Frame*>(glfwGetWindowUserPointer(handle));

                WindowResizedEvent event(width, height);
                frame.EventPassthrough(event);
            });

            glfwSetKeyCallback(frame.window, [](GLFWwindow* handle, int key, int scancode, int action, int mods)
            {
                Frame& frame = *static_cast<Frame*>(glfwGetWindowUserPointer(handle));

                switch (action)
                {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                    {
                        KeyPressedEvent event(key);
                        frame.EventPassthrough(event);
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        KeyReleasedEvent event(key);
                        frame.EventPassthrough(event);
                        break;
                    }
                    default:
                        assert(false);
                }
            });
        }
    }
}
