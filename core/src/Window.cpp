#include "Window.h"
#include "glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

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
            Window::Destroy(*this);
        }

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

            glfwSetWindowUserPointer(frame.window, &frame);

            glfwSetWindowCloseCallback(frame.window, [](GLFWwindow* handle)
            {
                Frame& frame = *((Frame*)glfwGetWindowUserPointer(handle));

                WindowClosedEvent event;
                frame.EventPassthrough(event);
            });
        }

        bool ShouldClose(const Frame &frame)
        {
            return glfwWindowShouldClose(frame.window) != 0;
        }

        void Destroy(Frame &frame)
        {
            if (frame.window)
            {
                glfwDestroyWindow(frame.window);
            }
            frame.window = nullptr;
        }

        void Update(Frame &frame)
        {
            glfwSwapBuffers(frame.window);
        }

    }
}
