#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Chinstrap::UserSettings
{
    enum class VSyncMode
    {
        OFF,
        ON, FAST
    };

    enum class ColorSpaceMode
    {
        SRGB, HDR
    };

    struct GraphicsSettings
    {
        VSyncMode vSync;

        ColorSpaceMode colorSpace;

        GraphicsSettings()
            : vSync(VSyncMode::ON), colorSpace(ColorSpaceMode::SRGB) {}
        GraphicsSettings(const VSyncMode vSync, const ColorSpaceMode colorSpace)
            : vSync(vSync), colorSpace(colorSpace) {}
    };

}