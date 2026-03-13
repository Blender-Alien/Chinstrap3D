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

    enum class FullScreenMode
    {
        OFF, ON
    };

    template<typename type>
    struct Setting
    {
        type desiredValue;
        type actualValue;

        explicit Setting(type initialValue)
            : desiredValue(initialValue), actualValue(initialValue) {}
    };

    struct GraphicsSettings
    {
        Setting<VSyncMode> vSync = Setting(VSyncMode::ON);
        Setting<ColorSpaceMode> colorSpace = Setting(ColorSpaceMode::SRGB);
        Setting<FullScreenMode> fullScreen = Setting(FullScreenMode::OFF);
    };

}