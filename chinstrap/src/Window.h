#pragma once
#include "events/Event.h"
#include "rendering/VulkanData.h"
#include "rendering/Renderer.h"
#include "UserSettings.h"

#include <functional>

class GLFWwindow;
class GLFWmonitor;

namespace Chinstrap::Display
{
    struct WindowSpec
    {
        // Arbitrary length, increase if more is needed
        char title[24] = "Chinstrap App";
        float dpiScale = 1.0f;
        int width = 1920;
        int height = 1080;

        explicit WindowSpec() = default;
        explicit WindowSpec(const char* title_arg, const int width_arg, const int height_arg)
            : width(width_arg), height(height_arg) { strcpy(title, title_arg); }
    };

    struct Window
    {
        std::function<void(Event&)> EventPassthrough;

        GLFWwindow *glfwWindow = nullptr;
        ChinVulkan::VulkanContext vulkanContext;
        Renderer::RenderContext renderContext;

        WindowSpec windowSpec;

        [[nodiscard]] bool ShouldClose() const;

        void Create(const WindowSpec &windowSpec_arg, UserSettings::GraphicsSettings &graphicsSettings_arg, const std::vector<std::unique_ptr<Scene>>& sceneStack_arg);
        void FinishRendering() const;
        void Destroy();

        explicit Window();
        Window(const Window&) = delete;
        Window &operator=(const Window&) = delete;
    };

}

