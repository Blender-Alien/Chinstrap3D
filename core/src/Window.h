#pragma once

#include "Event.h"

#include <string>
#include <cinttypes>
#include <functional>

class GLFWwindow;

namespace Chinstrap
{
    namespace Window
    {
        struct FrameSpec
        {
            FrameSpec(const std::string &title, const int &width, const int &height, bool isResizable,
                      bool vSync);

            std::string Title;
            int Width = 1280;
            int Height = 720;
            bool IsResizable = true;
            bool VSync = true;
        };

        struct Frame
        {
            Frame(const FrameSpec &spec);

            ~Frame();

            FrameSpec frameSpec;
            GLFWwindow *window = nullptr;

            std::function<void(Event&)> EventPassthrough;
        };

        void Create(Frame &frame);

        void Destroy(Frame &frame);

        void Update(Frame &frame);

        bool ShouldClose(const Frame &frame);
    }
}
