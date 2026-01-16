#pragma once

#include "src/Application.h"
#include <cstdint>
#include <string>
#include <inttypes.h>

namespace Chinstrap
{
    namespace Window {

        struct FrameSpec
        {
            FrameSpec(const std::string& title, const uint32_t& width, const uint32_t& height, bool isResizable, bool vSync);

            std::string Title;
            uint32_t Width = 1280;
            uint32_t Height = 720;
            bool IsResizable = true;
            bool VSync = true;
        };

        struct Frame
        {
            Frame(const FrameSpec& spec);
            ~Frame();

            FrameSpec frameSpec; 
            GLFWwindow* window = nullptr;
        };

        void Create(Frame& frame);
        void Destroy(Frame& frame);
        void Update(Frame& frame);
        bool ShouldClose(Frame& frame);
    }

}
