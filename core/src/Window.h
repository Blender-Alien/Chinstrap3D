#pragma once

#include "src/Application.h"
#include <cstdint>
#include <memory>
#include <string>
#include <inttypes.h>

namespace Chinstrap
{
    namespace Window {

        struct Frame
        {
            Frame(std::string title, uint32_t width, uint32_t height, bool isResizable, bool vSync);
            ~Frame();

            std::string Title;
            uint32_t Width = 1280;
            uint32_t Height = 720;
            bool IsResizable = true;
            bool VSync = true;

            GLFWwindow* window = nullptr;
        };

        std::unique_ptr<Frame> Create(std::string title, uint32_t width, uint32_t height, bool isResizable, bool vSync);
        void Destroy(Frame& frame);
        void Update(Frame& frame);
    }

}
