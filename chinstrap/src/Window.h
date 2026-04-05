#pragma once
#include "events/Event.h"
#include "rendering/VulkanData.h"
#include "rendering/Renderer.h"

class GLFWwindow;
class GLFWmonitor;

namespace Chinstrap {struct Scene;}

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
        WindowSpec(const char* title_arg, const int width_arg, const int height_arg)
            : width(width_arg), height(height_arg) { strcpy(title, title_arg); }
    };

    struct Window
    {
        void(*eventPassthrough)(Event& event);

        GLFWwindow *glfwWindow = nullptr;
        ChinVulkan::VulkanContext vulkanContext;
        Renderer::RenderContext renderContext;

        WindowSpec windowSpec;

        [[nodiscard]] bool ShouldClose() const;

        bool Create(const WindowSpec &windowSpec_arg, UserSettings::GraphicsSettings &graphicsSettings_arg, const std::vector<std::unique_ptr<Scene>>& sceneStack_arg);
        void FinishRendering() const;
        void Destroy();

        explicit Window();
        Window(const Window&) = delete;
        Window &operator=(const Window&) = delete;
    };

}

