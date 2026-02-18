#pragma once
#include "events/Event.h"
#include "rendering/VulkanData.h"
#include "UserSettings.h"

#include <string>
#include <functional>

class GLFWwindow;
class GLFWmonitor;

namespace Chinstrap::Window
{
    struct ViewPortSpec
    {
        float posScaleX;
        float posScaleY;
        float sizeScaleX;
        float sizeScaleY;

        explicit ViewPortSpec();
        void Create(float posScaleX, float posScaleY, float sizeScaleX, float sizeScaleY);
    };

    struct FrameSpec
    {
        std::string title;
        float dpiScale = 1.0f;
        int width = 1280;
        int height = 720;

        explicit FrameSpec();
        void Create(const std::string &title, int width, int height);
    };

    struct Frame
    {
        std::function<void(Event&)> EventPassthrough;

        GLFWwindow *window = nullptr;
        ChinVulkan::VulkanContext vulkanContext;

        FrameSpec frameSpec;
        ViewPortSpec viewPortSpec;
        UserSettings::GraphicsSettings graphicsSettings;

        explicit Frame();
        Frame(const Frame&) = delete;
        Frame &operator=(const Frame&) = delete;

        void Create(const FrameSpec &FrameSpec, const ViewPortSpec &ViewportSpec, const UserSettings::GraphicsSettings &GraphicsSettings);
        void Destroy();
    };

    void Update(const Frame &frame);

    bool ShouldClose(const Frame &frame);
}

