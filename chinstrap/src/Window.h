#pragma once

#include "Event.h"

#include <string>
#include <functional>

class GLFWwindow;
class GLFWmonitor;

namespace Chinstrap
{
    namespace Window
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
            bool isResizable;
            bool vSync = true;

            explicit FrameSpec(const std::string &title, int width, int height, bool isResizable,
                      bool vSync);
        };

        struct Frame
        {
            GLFWwindow *window = nullptr;
            GLFWmonitor *monitor = nullptr;
            FrameSpec frameSpec;
            ViewPortSpec viewPortSpec;
            std::function<void(Event&)> EventPassthrough;


            explicit Frame(const FrameSpec &spec, const ViewPortSpec &viewportSpec);
            ~Frame();
        };

        void Create(Frame &frame);

        void Destroy(Frame &frame);

        void Update(const Frame &frame);

        bool ShouldClose(const Frame &frame);
    }
}
