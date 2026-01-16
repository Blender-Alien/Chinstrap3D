#include "Window.h"
#include "GLFW/glfw3.h"

namespace Chinstrap
{
    namespace Window
    {
        Frame::Frame(std::string title, uint32_t width, uint32_t height, bool isResizable, bool vSync)
            : Title(title), Width(width), Height(height), IsResizable(isResizable), VSync(vSync)
        {
        }        

        Frame::~Frame()
        {
            Window::Destroy(*this);
        }

        void Create()
        {
            // Create Window
        }

        void Destroy(Frame& frame)
        {
            if (frame.window)
            {
                glfwDestroyWindow(frame.window);
            }
            frame.window = nullptr;
        }

        void Update(Frame& frame)
        {
            glfwSwapBuffers(frame.window);
        }
    }
}
