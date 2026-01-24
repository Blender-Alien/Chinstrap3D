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
        ViewPortSpec::ViewPortSpec(int posX, int posY, int width, int height)
            : posX(posX), posY(posY), width(width), height(height)
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

            glfwMakeContextCurrent(frame.window);

            if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
            {
                std::cerr << "Glad could not load!\n";
                assert(false);
            }

            glViewport(frame.viewPortSpec.posX, frame.viewPortSpec.posY, frame.viewPortSpec.width,
                       frame.viewPortSpec.height);

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
                const Frame &userFrame = *static_cast<Frame *>(glfwGetWindowUserPointer(handle));

                //TODO: Only do this once when new size is stable
                //TODO: are posX and posY actually pixel values???
                glViewport(
                    userFrame.viewPortSpec.posX * (width / userFrame.frameSpec.width),
                    userFrame.viewPortSpec.posX * (height / userFrame.frameSpec.height),
                    userFrame.viewPortSpec.width * (width / userFrame.frameSpec.width),
                    userFrame.viewPortSpec.height * (height / userFrame.frameSpec.height)
                );

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
}
