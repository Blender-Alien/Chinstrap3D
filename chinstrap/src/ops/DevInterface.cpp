#include "DevInterface.h"

#include "../Application.h"
#include "../Window.h"
#include "../rendering/VulkanFunctions.h"
#include "../rendering/Renderer.h"
#include "Roboto/Roboto-Regular.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_vulkan.cpp"
#include "imgui_internal.h"
#include "../Scene.h"
#include <vulkan/vulkan_core.h>

void Chinstrap::DevInterface::ContextInfo(float posScaleX, float posScaleY)
{
}

// Profile sceneStack function performance and overall FPS
void Chinstrap::DevInterface::PerformanceInfo(float posScaleX, float posScaleY)
{
    /*
    int x, y;
    glfwGetWindowSize(Application::App::Get().frame->window, &x, &y);
    ImGui::SetNextWindowPos(ImVec2(x * posScaleX, y * posScaleY));
    */

    ImGui::Begin("Performance");
    ImGui::Text("%d FPS", Application::App::Get().framerate);

    using namespace UserSettings;
    VSyncMode& setting = Application::App::Get().frame->graphicsSettings.vSync;

    bool vsync;
    switch (setting)
    {
        case VSyncMode::OFF:
            vsync = false;
            break;
        case VSyncMode::ON:
        case VSyncMode::FAST:
            vsync = true;
            break;
    }
    ImGui::Checkbox("VSync", &vsync);
    if (vsync && setting == VSyncMode::OFF) {
        setting = VSyncMode::ON;
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
    } else if (!vsync && setting == VSyncMode::ON)
    {
        setting = VSyncMode::OFF;
        ChinVulkan::RecreateSwapChain(*Application::App::Get().frame);
    }

    for (std::unique_ptr<Scene> &scene: Application::App::Get().sceneStack)
    {
        ImGui::Text("(%fms): OnUpdate() <- [%s]", scene->OnUpdateProfile, scene->GetName().c_str());
        ImGui::Text("(%fms): OnRender() <- [%s]", scene->OnRenderProfile, scene->GetName().c_str());
    }

    ImGui::End();
}

void Chinstrap::DevInterface::Render(){ Render([](){}); }
void Chinstrap::DevInterface::Render(void(*lambda)())
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    lambda();

    ImGui::Render();
}

void Chinstrap::DevInterface::Initialize(float fontSize)
{
    Window::Frame& frame = *Application::App::Get().frame;

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    poolInfo.pPoolSizes = pool_sizes;

    ChinVulkan::VulkanContext& context = frame.vulkanContext;
    if (vkCreateDescriptorPool(context.virtualGPU, &poolInfo, nullptr, &context.imguiPool) != VK_SUCCESS)
    {
        assert(false);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    ImGui_ImplGlfw_InitForVulkan(frame.window, true);

    ImGui_ImplVulkan_InitInfo info = {};
    info.ApiVersion = context.instanceSupportedVersion;
    info.Instance = context.instance;
    info.PhysicalDevice = context.physicalGPU;
    info.Device = context.virtualGPU;
    info.QueueFamily = VK_QUEUE_GRAPHICS_BIT;
    info.Queue = context.graphicsQueue;
    info.DescriptorPool = context.imguiPool;
    info.MinImageCount = 3;
    info.ImageCount = 3;
    info.UseDynamicRendering = true;

    info.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &context.swapChainImageFormat;
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&info);

    float xscale, yscale;
    glfwGetWindowContentScale(Application::App::Get().frame->window, &xscale, &yscale);

    ImGui::GetStyle().FontScaleDpi = xscale;
    ImFontConfig font_config;
    font_config.RasterizerDensity = xscale;
    font_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(robotoRegular, sizeof(robotoRegular), fontSize, &font_config);
    io.Fonts->Build();

}
void Chinstrap::DevInterface::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
