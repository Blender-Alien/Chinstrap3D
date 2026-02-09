#include "VulkanFunctions.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../ops/Logging.h"
#include "../Window.h"
#include "VulkanData.h"

#include <set>
#include <limits>
#include <algorithm>
#include <any>

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
                    indices.graphicsFamily = index;
                }
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.presentationFamily = index;
                }

                if (indices.allSupported()) { break; }
                ++index;
            }

            return indices;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats,
                                                   UserSettings::GraphicsSettings &settings)
        {
            // TODO: Find out which ones we need for HDR
            std::array<VkColorSpaceKHR, 3> supportedHDRSpaces = {
                VK_COLOR_SPACE_HDR10_ST2084_EXT,
                VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
                VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
            };
            using namespace UserSettings;
            for (const auto &availableFormat: availableFormats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                    && settings.colorSpace == ColorSpaceMode::SRGB)
                {
                    return availableFormat;
                }
                if (std::find(std::begin(supportedHDRSpaces), std::end(supportedHDRSpaces), availableFormat.colorSpace) !=
                    std::end(supportedHDRSpaces)
                    && settings.colorSpace == ColorSpaceMode::HDR)
                {
                    return availableFormat;
                }
            }
            if (settings.colorSpace != ColorSpaceMode::SRGB)
            {
                CHIN_LOG_ERROR_VULKAN("There was no suitable HDR format available!");
                settings.colorSpace = ColorSpaceMode::SRGB;
            }
            CHIN_LOG_ERROR_VULKAN("Chose fallback format and colorspace!");
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

//* Externally Available Functions *//

bool Chinstrap::ChinVulkan::Init(VulkanContext &vulkanContext, const std::string &name)
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
    /* We want to use dynamic rendering, which is only supported in Vulkan 1_3 upwards or in Vulkan 1_2 as an extension*/
    if (vulkanContext.instanceSupportedVersion >= VK_API_VERSION_1_3)
    {
        appInfo.apiVersion = VK_API_VERSION_1_3;
    } else if (vulkanContext.instanceSupportedVersion >= VK_API_VERSION_1_2 && vulkanContext.instanceSupportedVersion <
               VK_API_VERSION_1_3)
    {
        appInfo.apiVersion = VK_API_VERSION_1_2;
        vulkanContext.neededDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    } else
    {
        CHIN_LOG_CRITICAL("Dynamic rendering is not supported!");
        return false;
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef CHIN_VK_VAL_LAYERS
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &vulkanContext.instance) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create instance!!!");
        return false;
    }

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

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU // Arbitrary
            && deviceFeatures.geometryShader // Arbitrary
            && queues.graphicsFamily == queues.presentationFamily // Arbitrary
            && queues.allSupported() // Necessary
            && extensionsSupported // Necessary
            && swapChainAdequate) // Necessary
        {
            vulkanContext.physicalGPU = device;
            CHIN_LOG_INFO_VULKAN("Successfully chose a GPU");
            return true;
        }
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
    if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
    {
        dynamicRenderingFeatures.dynamicRendering = true;
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &dynamicRenderingFeatures;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanContext.neededDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = vulkanContext.neededDeviceExtensions.data();

    if (vkCreateDevice(vulkanContext.physicalGPU, &deviceCreateInfo, nullptr, &vulkanContext.virtualGPU) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create a Vulkan virtual GPU!");
        return false;
    }

    vkGetDeviceQueue(vulkanContext.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanContext.graphicsQueue);
    CHIN_LOG_INFO_VULKAN("Successfully created virtual GPU");
    return true;
}

bool Chinstrap::ChinVulkan::CreateSyncObjects(VulkanContext &vulkanContext)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(vulkanContext.virtualGPU, &semaphoreCreateInfo, nullptr,
                          &vulkanContext.imageAvailableSemaphore)
        != VK_SUCCESS ||
        vkCreateSemaphore(vulkanContext.virtualGPU, &semaphoreCreateInfo, nullptr,
                          &vulkanContext.renderFinishedSemaphore)
        != VK_SUCCESS ||
        vkCreateFence(vulkanContext.virtualGPU, &fenceCreateInfo, nullptr, &vulkanContext.inFlightFence)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create semaphores!");
        return false;
    }
    return true;
}

void Chinstrap::ChinVulkan::ExampleCreateMaterial(const VulkanContext &vulkanContext, Material &material)
{
    CHIN_LOG_INFO_VULKAN_F("FragShader size: {0}; VertShader size: {1}", material.fragmentShaderCode.size(),
                           material.vertexShaderCode.size());

    VkShaderModule vertShaderModule = CreateShaderModule(vulkanContext, material.vertexShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(vulkanContext, material.fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    // Hard coding vertex info in vertex shader for now, thus no input
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vulkanContext.swapChainExtent.width);
    viewport.height = static_cast<float>(vulkanContext.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vulkanContext.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.0f;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(vulkanContext.virtualGPU, &pipelineLayoutCreateInfo, nullptr, &material.pipelineLayout)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create pipeline layout!");
        assert(false);
    }

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.pNext = VK_NULL_HANDLE;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &material.vulkanContext.swapChainImageFormat;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
    graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = material.pipelineLayout;

    if (vkCreateGraphicsPipelines(vulkanContext.virtualGPU, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &material.pipeline)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics pipeline!");
        assert(false);
    }

    CHIN_LOG_INFO_VULKAN("Successfully created barebones material");

    vkDestroyShaderModule(vulkanContext.virtualGPU, vertShaderModule, nullptr);
    vkDestroyShaderModule(vulkanContext.virtualGPU, fragShaderModule, nullptr);
}

void Chinstrap::ChinVulkan::ExampleCreateImageViews(const VulkanContext &vulkanContext, std::vector<VkImageView> &imageViews)
{
    imageViews.resize(vulkanContext.swapChainImages.size());

    for (size_t i = 0; i < vulkanContext.swapChainImages.size(); i++)
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

        if (vkCreateImageView(vulkanContext.virtualGPU, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create Vulkan image view!");
            assert(false);
        }
    }
    CHIN_LOG_INFO_VULKAN("Successfully created ImageViews");
}

VkCommandPool Chinstrap::ChinVulkan::ExampleCreateCommandPool(const VulkanContext &vulkanContext)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkanContext.physicalGPU, vulkanContext.windowSurface);

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkCommandPool commandPool;

    if (vkCreateCommandPool(vulkanContext.virtualGPU, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command pool!");
    }
    return commandPool;
}

VkCommandBuffer Chinstrap::ChinVulkan::ExampleCreateCommandBuffer(const VulkanContext &vulkanContext, const VkCommandPool &commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;

    if (vkAllocateCommandBuffers(vulkanContext.virtualGPU, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command buffer!");
    }
    return commandBuffer;
}

void Chinstrap::ChinVulkan::ExampleRecordCommandBuffer(VkCommandBuffer &targetCommandBuffer, uint32_t imageIndex, const Restaurant &restaurant,
                                const Material &material, const VulkanContext &vulkanContext)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(targetCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to begin recording command buffer!");
    }

    VkRenderingAttachmentInfo colorAttachmentInfo = {};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.imageView = restaurant.swapChainImageViews[imageIndex];

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;
    renderInfo.renderArea.offset = {0, 0};
    renderInfo.renderArea.extent = restaurant.vulkanContext.swapChainExtent;

    if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
        vulkanContext.PFN_vkCmdBeginRenderingKHR(targetCommandBuffer, &renderInfo);
    else
        vkCmdBeginRendering(targetCommandBuffer, &renderInfo);

    vkCmdBindPipeline(targetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.pipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(restaurant.vulkanContext.swapChainExtent.width);
    viewport.height = static_cast<float>(restaurant.vulkanContext.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(targetCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = restaurant.vulkanContext.swapChainExtent;
    vkCmdSetScissor(targetCommandBuffer, 0, 1, &scissor);

    vkCmdDraw(targetCommandBuffer, 3, 1, 0, 0);

    if (vulkanContext.instanceSupportedVersion < VK_API_VERSION_1_3)
        vulkanContext.PFN_vkCmdEndRenderingKHR(targetCommandBuffer);
    else
        vkCmdEndRendering(targetCommandBuffer);

    if (vkEndCommandBuffer(targetCommandBuffer) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR_VULKAN("Failed to end recording command buffer!");
    }
}

void Chinstrap::ChinVulkan::Shutdown(VulkanContext &vulkanContext)
{
    vkDestroySemaphore(vulkanContext.virtualGPU, vulkanContext.imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(vulkanContext.virtualGPU, vulkanContext.renderFinishedSemaphore, nullptr);
    vkDestroyFence(vulkanContext.virtualGPU, vulkanContext.inFlightFence, nullptr);

    vkDestroySwapchainKHR(vulkanContext.virtualGPU, vulkanContext.swapChain, nullptr);
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
