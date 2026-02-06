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

        explicit ViewPortSpec(float posScaleX, float posScaleY, float sizeScaleX, float sizeScaleY);
    };

    struct FrameSpec
    {
        std::string title;
        float dpiScale = 1.0f;
        int width = 1280;
        int height = 720;

        explicit FrameSpec(const std::string &title, int width, int height);
    };

    struct Frame
    {
        std::function<void(Event&)> EventPassthrough;
        FrameSpec frameSpec;
        ViewPortSpec viewPortSpec;

        GLFWwindow *window = nullptr;
        GLFWmonitor *monitor = nullptr;
        ChinVulkan::VulkanContext vulkanContext;

        UserSettings::GraphicsSettings graphicsSettings;

        Frame(const Frame&) = delete;
        Frame &operator=(const Frame&) = delete;

        explicit Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec);
        explicit Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec, const UserSettings::GraphicsSettings &graphicsSettings);
        ~Frame();
    };

    void Create(Frame &frame);

    void Destroy(Frame &frame);

    void Update(const Frame &frame);

    bool ShouldClose(const Frame &frame);
}

