#pragma once
#include "events/Event.h"
#include "rendering/VulkanData.h"

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
        bool vSync = true;
        bool isResizable;

        explicit FrameSpec(const std::string &title, int width, int height, bool isResizable, bool vSync);
    };

    struct Frame
    {
        std::function<void(Event&)> EventPassthrough;
        FrameSpec frameSpec;
        ViewPortSpec viewPortSpec;

        GLFWwindow *window = nullptr;
        GLFWmonitor *monitor = nullptr;

        ChinVulkan::VulkanSetupData vulkanSetupData;

        Frame(const Frame&) = delete;
        Frame &operator=(const Frame&) = delete;

        explicit Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec);
        ~Frame();
    };

    void Create(Frame &frame);

    void Destroy(Frame &frame);

    void Update(const Frame &frame);

    bool ShouldClose(const Frame &frame);
}
