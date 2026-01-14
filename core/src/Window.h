#pragma once

#include <string>
#include <inttypes.h>

namespace Chinstrap
{
    struct WindowSpecification
    {
        std::string Title;

        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool IsResizable = true;
        bool VSync = true;

    };

}
