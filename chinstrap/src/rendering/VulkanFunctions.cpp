#include "VulkanFunctions.h"

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_vulkan.h"

#include "Renderer.h"
#include "VulkanData.h"
#include "../ops/Logging.h"
#include "../Window.h"

#include <set>
#include <limits>
#include <algorithm>

#include "glm/vector_relational.hpp"

//* Externally Unavailable Functions *//
namespace Chinstrap::ChinVulkan
{
    namespace
    {
        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &physicalGPU,
                                                      const VkSurfaceKHR &windowSurface)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalGPU, windowSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalGPU, windowSurface, &formatCount, nullptr);
            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalGPU, windowSurface, &formatCount, details.formats.data());
            }

            uint32_t presentationModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalGPU, windowSurface, &presentationModeCount, nullptr);
            if (presentationModeCount != 0)
            {
                details.presentModes.resize(presentationModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalGPU, windowSurface, &presentationModeCount,
                                                          details.presentModes.data());
            }

            return details;
        }

        QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physicalGPU, const VkSurfaceKHR &windowSurface)
        {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalGPU, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalGPU, &queueFamilyCount, queueFamilies.data());

            VkBool32 presentationSupport = false;

            int index = 0;
            for (const auto &queueFamily: queueFamilies)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalGPU, index, windowSurface, &presentationSupport);
                if (presentationSupport)
                {
                    indices.presentationFamily = index;
                }
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.graphicsFamily = index;
                }

                if (indices.allSupported()) { break; }
                ++index;
            }

            return indices;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats,
                                                   UserSettings::GraphicsSettings &settings)
        {
            std::array<VkColorSpaceKHR, 3> supportedHDRSpaces = {
                VK_COLOR_SPACE_HDR10_ST2084_EXT,
                VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
                VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
            };
            using namespace UserSettings;
            for (const auto &availableFormat: availableFormats)
            {
                if (settings.colorSpace == ColorSpaceMode::SRGB
                    && availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
                    && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return availableFormat;
                }
                if (settings.colorSpace == ColorSpaceMode::HDR
                    && (std::find(std::begin(supportedHDRSpaces), std::end(supportedHDRSpaces), availableFormat.colorSpace)
                    != std::end(supportedHDRSpaces)))
                {
                    return availableFormat;
                }
            }

            if (settings.colorSpace != ColorSpaceMode::SRGB)
            {
                CHIN_LOG_ERROR_VULKAN("No HDR format found, switching to SRGB!");
            #ifdef __linux
                CHIN_LOG_WARN_VULKAN("On Wayland, gamescope is required to use HDR");
            #endif
                settings.colorSpace = ColorSpaceMode::SRGB;
            }

            CHIN_LOG_ERROR_VULKAN("Choosing first available format as fallback!");
            return availableFormats[0]; // Whatever Format is actually supported, no matter which
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes,
                                               UserSettings::GraphicsSettings &settings)
        {
            using namespace UserSettings;
            for (const auto &availablePresentMode: availablePresentModes)
            {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR
                    && settings.vSync == VSyncMode::FAST)
                {
                    return availablePresentMode;
                }
                if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR
                    && settings.vSync == VSyncMode::OFF)
                {
                    return availablePresentMode;
                }
            }
            if (settings.vSync != VSyncMode::ON)
            {
                CHIN_LOG_ERROR_VULKAN("Selected VSync Mode is not supported!");
                settings.vSync = VSyncMode::ON; // Only this mode is guaranteed to be available
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window::Frame &frame,
                                    UserSettings::GraphicsSettings &settings)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }

            int width, height;
            glfwGetFramebufferSize(frame.window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(width)
            };
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}

bool Chinstrap::ChinVulkan::Initialize(Window::Frame &frame)
{
    if (   !ChinVulkan::CreateContext(frame.vulkanContext, frame.frameSpec.title) 
        || !ChinVulkan::CreateSurface(frame)
        || !ChinVulkan::AutoPickPhysicalGPU(frame.vulkanContext)
        || !ChinVulkan::CreateVirtualGPU(frame.vulkanContext)
        || !ChinVulkan::CreateVMA(frame.vulkanContext)
        || !ChinVulkan::CreateSwapChain(frame)
        || !ChinVulkan::CreateDefaultImageViews(frame.vulkanContext))
    {
        return false;
    }
    return true;
}

bool Chinstrap::ChinVulkan::CreateContext(VulkanContext &vulkanContext, const std::string &name)
{
    uint32_t instanceVersion;
    vkEnumerateInstanceVersion(&instanceVersion);
    CHIN_LOG_INFO_VULKAN_F("Supported Instance Version: {}.{}", VK_API_VERSION_MAJOR(instanceVersion),
                           VK_API_VERSION_MINOR(instanceVersion));
    vulkanContext.instanceSupportedVersion = instanceVersion;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = "Chinstrap3D";
    if (vulkanContext.instanceSupportedVersion >= VK_API_VERSION_1_3)
    {
        appInfo.apiVersion = VK_API_VERSION_1_3;
    } else if (vulkanContext.instanceSupportedVersion >= VK_API_VERSION_1_2 && vulkanContext.instanceSupportedVersion <
               VK_API_VERSION_1_3)
    {
        appInfo.apiVersion = VK_API_VERSION_1_2;
    } else
    {
        CHIN_LOG_CRITICAL("Dynamic rendering is not supported!");
        return false;
    }
    vulkanContext.neededDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef CHIN_VK_VAL_LAYERS
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef CHIN_VK_VAL_LAYERS
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool layerFound = false;
    for (const char *layerName: vulkanContext.validationLayers)
    {
        layerFound = false;
        for (const auto &layerProperties: availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            break;
    }
    if (!layerFound)
    {
        CHIN_LOG_CRITICAL_VULKAN("Requested validation Layers, but they are not available!");
    }
    createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanContext.validationLayers.size());
    createInfo.ppEnabledLayerNames = vulkanContext.validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif
    if (vkCreateInstance(&createInfo, nullptr, &vulkanContext.instance) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create instance!!!");
        return false;
    }

    //* Setup Debug logging callback *//
#ifdef CHIN_VK_VAL_LAYERS
    VkDebugUtilsMessengerCreateInfoEXT createMessengerInfo{};
    createMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createMessengerInfo.pfnUserCallback = debugCallback;
    createMessengerInfo.pUserData = nullptr;

    // Create DebugUtilsMessangerEXT
    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        vulkanContext.instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(vulkanContext.instance, &createMessengerInfo, nullptr, &vulkanContext.debugMessenger);
    } else
    {
        CHIN_LOG_ERROR_VULKAN("Failed to set up Vulkan debug messenger!");
    }
#endif

    if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
    {
        const auto begin = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetInstanceProcAddr(
            vulkanContext.instance, "vkCmdBeginRenderingKHR"));
        const auto end = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetInstanceProcAddr(
            vulkanContext.instance, "vkCmdEndRenderingKHR"));

        if (begin == nullptr || end == nullptr)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to load DynamicRendering Extension functions, but they are required!");
            return false;
        }
        vulkanContext.PFN_vkCmdBeginRenderingKHR = begin;
        vulkanContext.PFN_vkCmdEndRenderingKHR = end;
    }

    CHIN_LOG_INFO_VULKAN("Initialized");
    return true;
}

bool Chinstrap::ChinVulkan::CreateSurface(Window::Frame &frame)
{
    if (glfwCreateWindowSurface(frame.vulkanContext.instance, frame.window, nullptr, &frame.vulkanContext.windowSurface)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN_F("Failed to create rendering-surface on window: {}", frame.frameSpec.title);
        return false;
    }
    CHIN_LOG_INFO_VULKAN("Successfully created rendering surface");
    return true;
}

bool Chinstrap::ChinVulkan::CreateSwapChain(Window::Frame &frame)
{
    SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(
        frame.vulkanContext.physicalGPU, frame.vulkanContext.windowSurface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats, frame.graphicsSettings);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes, frame.graphicsSettings);
    VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities, frame, frame.graphicsSettings);

    CHIN_LOG_INFO_VULKAN_F("SwapChain Info: Res: {0}x{1}", extent.width, extent.height);

    uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
    if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.
        maxImageCount)
    {
        imageCount = swapChainSupportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = frame.vulkanContext.windowSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // We're not doing VR/3D
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(frame.vulkanContext.physicalGPU, frame.vulkanContext.windowSurface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentationFamily.value()};
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform; // No Transformations
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(frame.vulkanContext.virtualGPU, &createInfo, nullptr, &frame.vulkanContext.swapChain) !=
        VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create SwapChain!");
        return false;
    }

    vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount, nullptr);
    frame.vulkanContext.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount,
                            frame.vulkanContext.swapChainImages.data());

    frame.vulkanContext.swapChainImageFormat = surfaceFormat.format;
    frame.vulkanContext.swapChainExtent = extent;
    CHIN_LOG_INFO_VULKAN("Successfully created SwapChain");
    return true;
}


bool Chinstrap::ChinVulkan::RecreateSwapChain(Window::Frame &frame)
{
    vkDeviceWaitIdle(frame.vulkanContext.virtualGPU);

    CleanupSwapChain(frame.vulkanContext);
    if (!CreateSwapChain(frame) || !CreateDefaultImageViews(frame.vulkanContext))
    {
        return false;
    }
    return true;
}

void Chinstrap::ChinVulkan::CleanupSwapChain(VulkanContext &vulkanContext)
{
    for (auto &imageView: vulkanContext.defaultImageViews)
    {
        vkDestroyImageView(vulkanContext.virtualGPU, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vulkanContext.virtualGPU, vulkanContext.swapChain, nullptr);
}

// TODO: Verify that we make the right choice on Handhelds like SteamDeck
bool Chinstrap::ChinVulkan::AutoPickPhysicalGPU(VulkanContext &vulkanContext)
{
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to find any GPU with Vulkan support!");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, devices.data());

    VkPhysicalDevice suitableIntegratedGPU = VK_NULL_HANDLE;

    uint32_t extensionCount;
    bool extensionsSupported, swapChainAdequate;
    for (const auto &device: devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(vulkanContext.neededDeviceExtensions.begin(),
                                                 vulkanContext.neededDeviceExtensions.end());
        for (const auto &extension: availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        if ((extensionsSupported = requiredExtensions.empty()))
        {
            SwapChainSupportDetails swapChainDetails = querySwapChainSupport(device, vulkanContext.windowSurface);
            swapChainAdequate = (!swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty());
        }

        QueueFamilyIndices queues = findQueueFamilies(device, vulkanContext.windowSurface);

        if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
            || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            && deviceFeatures.geometryShader // Arbitrary
            && queues.graphicsFamily == queues.presentationFamily // Arbitrary
            && queues.allSupported() // Necessary
            && extensionsSupported // Necessary
            && swapChainAdequate) // Necessary
        {
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                suitableIntegratedGPU = device;
                continue; // Don't immediately settle, there could be a discrete GPU left
            }
            /* Force integrated GPU for testing purposes
            vulkanContext.physicalGPU = device;
            CHIN_LOG_INFO_VULKAN("Successfully chose a dedicated GPU");
            return true;
            */
        }
    }
    if (suitableIntegratedGPU != VK_NULL_HANDLE)
    {
        vulkanContext.physicalGPU = suitableIntegratedGPU;
        CHIN_LOG_INFO("Successfully chose an integrated GPU");
        return true;
    }

    CHIN_LOG_CRITICAL_VULKAN("Failed to choose a GPU that meets requirements!");
    return false;
}

bool Chinstrap::ChinVulkan::CreateVirtualGPU(VulkanContext &vulkanContext)
{
    QueueFamilyIndices indices = findQueueFamilies(vulkanContext.physicalGPU, vulkanContext.windowSurface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentationFamily.value()};
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    float queuePriority = 1.0f;

    for (uint32_t queueFamily: uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = true;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &dynamicRenderingFeatures;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanContext.neededDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = vulkanContext.neededDeviceExtensions.data();

    if (vkCreateDevice(vulkanContext.physicalGPU, &deviceCreateInfo, nullptr, &vulkanContext.virtualGPU) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create a Vulkan virtual GPU!");
        return false;
    }

    vkGetDeviceQueue(vulkanContext.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanContext.graphicsQueue);
    vkGetDeviceQueue(vulkanContext.virtualGPU, indices.presentationFamily.value(), 0, &vulkanContext.presentationQueue);
    CHIN_LOG_INFO_VULKAN("Successfully created virtual GPU");
    return true;
}

bool Chinstrap::ChinVulkan::CreateVMA(VulkanContext &vulkanContext)
{
    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = vulkanContext.instanceSupportedVersion;
    allocatorCreateInfo.instance = vulkanContext.instance;
    allocatorCreateInfo.physicalDevice = vulkanContext.physicalGPU;
    allocatorCreateInfo.device = vulkanContext.virtualGPU;

    if (vmaCreateAllocator(&allocatorCreateInfo, &vulkanContext.allocator) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL("Failed to create vulkan memory allocator!");
        return false;
    }
    return true;
}


bool Chinstrap::ChinVulkan::CreateDefaultImageViews(VulkanContext &vulkanContext)
{
    vulkanContext.defaultImageViews.resize(vulkanContext.swapChainImages.size());

    for (size_t i = 0; i < vulkanContext.swapChainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vulkanContext.swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vulkanContext.swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vulkanContext.virtualGPU, &createInfo, nullptr, &vulkanContext.defaultImageViews[i]) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create Vulkan image view!");
            return false;
        }
    }
    CHIN_LOG_INFO_VULKAN("Successfully created default ImageViews");
    return true;
}

void Chinstrap::ChinVulkan::CreateCommandPool(const VulkanContext &vulkanContext, VkCommandPool* commandPool)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkanContext.physicalGPU, vulkanContext.windowSurface);

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(vulkanContext.virtualGPU, &poolCreateInfo, nullptr, commandPool) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command pool!");
    }
}

void Chinstrap::ChinVulkan::CreateCommandBuffer(const VulkanContext &vulkanContext, VkCommandBuffer* buffer, VkCommandPool* commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = *commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vulkanContext.virtualGPU, &commandBufferAllocateInfo, buffer) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command buffer!");
    }
}

void Chinstrap::ChinVulkan::ExampleRecordCommandBuffer(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext, const VkPipeline& pipeline)
{
    BeginRendering(targetCommandBuffer, vulkanContext, pipeline);

    vkCmdDraw(targetCommandBuffer, 3, 1, 0, 0);

    EndRendering(targetCommandBuffer, vulkanContext);
}

void Chinstrap::ChinVulkan::BeginRendering(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext,
                                           const VkPipeline& pipeline)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer(targetCommandBuffer, 0);
    if (vkBeginCommandBuffer(targetCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to begin recording command buffer!");
    }
    {
        VkImageMemoryBarrier waitImageMemoryBarrier = {};
        waitImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        waitImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        waitImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        waitImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        waitImageMemoryBarrier.image = vulkanContext.swapChainImages.at(Renderer::RenderContext::GetImageIndex());
        waitImageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        vkCmdPipelineBarrier(
            targetCommandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &waitImageMemoryBarrier
        );
    }
    {
        VkRenderingAttachmentInfo colorAttachmentInfo = {};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        colorAttachmentInfo.clearValue = clearColor;
        colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo.imageView = vulkanContext.defaultImageViews.at(Renderer::RenderContext::GetImageIndex());
        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachmentInfo;
        renderInfo.renderArea.offset = {0, 0};
        renderInfo.renderArea.extent = vulkanContext.swapChainExtent;

        if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
            vulkanContext.PFN_vkCmdBeginRenderingKHR(targetCommandBuffer, &renderInfo);
        else
            vkCmdBeginRendering(targetCommandBuffer, &renderInfo);

        // Get pipeline from material resource system
        vkCmdBindPipeline(targetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
    {
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanContext.swapChainExtent.width);
        viewport.height = static_cast<float>(vulkanContext.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(targetCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = vulkanContext.swapChainExtent;
        vkCmdSetScissor(targetCommandBuffer, 0, 1, &scissor);
    }
}
void Chinstrap::ChinVulkan::EndRendering(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext)
{
    if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
        vulkanContext.PFN_vkCmdEndRenderingKHR(targetCommandBuffer);
    else
        vkCmdEndRendering(targetCommandBuffer);
    {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.image = vulkanContext.swapChainImages.at(Renderer::RenderContext::GetImageIndex());
        imageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        vkCmdPipelineBarrier(targetCommandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier
        );
    }
    if (vkEndCommandBuffer(targetCommandBuffer) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to end recording command buffer!");
    }
}

void Chinstrap::ChinVulkan::ExampleRecordDevInterfaceCommandBuffer(VkCommandBuffer& targetCommandBuffer, const VulkanContext& vulkanContext)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer(targetCommandBuffer, 0);
    if (vkBeginCommandBuffer(targetCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to begin recording command buffer!");
    }
    {
        VkImageMemoryBarrier waitImageMemoryBarrier = {};
        waitImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        waitImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        waitImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        waitImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        waitImageMemoryBarrier.image = vulkanContext.swapChainImages.at(Renderer::RenderContext::GetImageIndex());
        waitImageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        vkCmdPipelineBarrier(
            targetCommandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &waitImageMemoryBarrier
        );
    }
    {
        VkRenderingAttachmentInfo colorAttachmentInfo = {};
        colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        colorAttachmentInfo.clearValue = clearColor;
        colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo.imageView = vulkanContext.defaultImageViews.at(Renderer::RenderContext::GetImageIndex());
        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachmentInfo;
        renderInfo.renderArea.offset = {0, 0};
        renderInfo.renderArea.extent = vulkanContext.swapChainExtent;

        if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
            vulkanContext.PFN_vkCmdBeginRenderingKHR(targetCommandBuffer, &renderInfo);
        else
            vkCmdBeginRendering(targetCommandBuffer, &renderInfo);
    }

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), targetCommandBuffer);

    EndRendering(targetCommandBuffer, vulkanContext);
}

void Chinstrap::ChinVulkan::Shutdown(VulkanContext &vulkanContext)
{

    CleanupSwapChain(vulkanContext);
    vmaDestroyAllocator(vulkanContext.allocator);
    vkDestroyDevice(vulkanContext.virtualGPU, nullptr);
#ifdef CHIN_VK_VAL_LAYERS
    const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        vulkanContext.instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(vulkanContext.instance, vulkanContext.debugMessenger, nullptr);
    } else
    {
        CHIN_LOG_ERROR_VULKAN("Failed to destroy debug messenger!");
    }
#endif
    vkDestroySurfaceKHR(vulkanContext.instance, vulkanContext.windowSurface, nullptr);
    vkDestroyInstance(vulkanContext.instance, nullptr);
    CHIN_LOG_INFO_VULKAN("Successfully shut a context down");
}
