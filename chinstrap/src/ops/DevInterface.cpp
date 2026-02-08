#include "DevInterface.h"

#include "../Application.h"
#include "../Window.h"
#include "../rendering/VulkanFunctions.h"
#include "Roboto/Roboto-Regular.h"

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.cpp"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_vulkan.cpp"
#include "imgui_internal.h"
#include "../Scene.h"

void Chinstrap::DevInterface::ContextInfo(float posScaleX, float posScaleY)
{
}

// For simple fps counter use different method
void Chinstrap::DevInterface::PerformanceInfo(float posScaleX, float posScaleY)
{
#ifdef CHIN_DEBUG
    //int x, y;
    //glfwGetWindowSize(Application::App::Get().frame->window, &x, &y);
    //ImGui::SetNextWindowPos(ImVec2(x * posScaleX, y * posScaleY));

    ImGui::Begin("Performance");
    ImGui::Text("%d FPS", Application::App::Get().framerate);

    //ImGui::Checkbox("VSync", &Application::App::Get().frame->frameSpec.vSync);
    //glfwSwapInterval(Application::App::Get().frame->frameSpec.vSync? 1 : 0);

    for (std::unique_ptr<Scene> &scene: Application::App::Get().sceneStack)
    {
        ImGui::Text("(%fms): OnUpdate() <- [%s]", scene->OnUpdateProfile, scene->GetName().c_str());
        ImGui::Text("(%fms): OnRender() <- [%s]", scene->OnRenderProfile, scene->GetName().c_str());
    }

    ImGui::End();
#endif
}

void Chinstrap::DevInterface::Render(){ Render([](){}); }
void Chinstrap::DevInterface::Render(void(*lambda)())
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    lambda();

    ImGui::Render();
    /* TODO: Need to figure out where to place this in the "graphics pipeline"
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), SomeCommandBuffer);
    */
}

void Chinstrap::DevInterface::Initialize(float fontSize)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    Window::Frame& frame = *Application::App::Get().frame;
    ImGui_ImplGlfw_InitForVulkan(frame.window, true);

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
    poolInfo.poolSizeCount = std::size(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;

    ChinVulkan::VulkanContext& context = frame.vulkanContext;

    VkDescriptorPool imguiPool;
    if (vkCreateDescriptorPool(context.virtualGPU, &poolInfo, nullptr, &imguiPool) != VK_SUCCESS)
    {
        assert(false);
    }

    ImGui_ImplVulkan_InitInfo info = {};
    info.Instance = context.instance;
    info.PhysicalDevice = context.physicalGPU;
    info.Device = context.virtualGPU;
    info.Queue = context.graphicsQueue;
    info.DescriptorPool = imguiPool;
    info.MinImageCount = 3;
    info.ImageCount = 3;

    ImGui_ImplVulkan_Init(&info);

    vkDestroyDescriptorPool(context.virtualGPU, imguiPool, nullptr);

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
