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
#include <ppltasks.h>

namespace Chinstrap::ChinVulkan
{
    namespace //* Externally Unavailable Functions *//
    {
        SwapChainSupportDetails querySwapChainSupport(VulkanContext& vulkanContext)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanContext.physicalGPU, vulkanContext.renderSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalGPU, vulkanContext.renderSurface, &formatCount, nullptr);
            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalGPU, vulkanContext.renderSurface, &formatCount, details.formats.data());
            }

            uint32_t presentationModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext.physicalGPU, vulkanContext.renderSurface, &presentationModeCount, nullptr);
            if (presentationModeCount != 0)
            {
                details.presentModes.resize(presentationModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext.physicalGPU, vulkanContext.renderSurface, &presentationModeCount, details.presentModes.data());
            }

            return details;
        }

        QueueFamilyIndices findQueueFamilies(VulkanContext& vulkanContext)
        {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext.physicalGPU, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext.physicalGPU, &queueFamilyCount, queueFamilies.data());

            VkBool32 presentationSupport = false;

            int index = 0;
            for (const auto &queueFamily: queueFamilies)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(vulkanContext.physicalGPU, index, vulkanContext.renderSurface, &presentationSupport);
                if (presentationSupport)
                {
                    indices.graphicsFamily = index;
                }
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.presentationFamily = index;
                }

                if (indices.isComplete()) {break;}
                ++index;
            }

            return indices;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, UserSettings::GraphicsSettings& settings)
        {
            using namespace UserSettings;
            for (const auto& availableFormat: availableFormats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                    && settings.colorSpace == ColorSpaceMode::SRGB)
                {
                    return availableFormat;
                }
                // TODO: Accommodate all the relevant HDR formats
                if (availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_HLG_EXT || availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT
                    && settings.colorSpace == ColorSpaceMode::HDR)
                {
                    return availableFormat;
                }
            }
            settings.colorSpace = UserSettings::ColorSpaceMode::SRGB; // There was no available HDR format
            return availableFormats[0]; // Whatever Format is actually supported, no matter how crap it is
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, UserSettings::GraphicsSettings& settings)
        {
            using namespace UserSettings;
            for (const auto& availablePresentMode: availablePresentModes)
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
                CHIN_LOG_ERROR("Selected VSync Mode is not supported!");
                settings.vSync = VSyncMode::ON; // Only this mode is guaranteed to be available
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window::Frame& frame, UserSettings::GraphicsSettings& settings)
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

    //* Externally Available Functions *//

    void Init(VulkanContext &vulkanContext, const std::string& name)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        CHIN_LOG_INFO("{} extensions supported!", extensionCount);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Chinstrap3D";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

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
            CHIN_LOG_CRITICAL("Failed to create Vulkan Instance!!!");
            assert(false);
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
            CHIN_LOG_CRITICAL("Requested Vulkan Validation Layers, but they are not available!");
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
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createMessengerInfo.pfnUserCallback = debugCallback;
        createMessengerInfo.pUserData = nullptr;

        // Create DebugUtilsMessangerEXT
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(vulkanContext.instance, &createMessengerInfo, nullptr, &vulkanContext.debugMessenger);
        } else {
            CHIN_LOG_ERROR("Failed to set up Vulkan debug messenger!");
        }
#endif
    }

    void Shutdown(VulkanContext& vulkanContext)
    {
        for (auto imageView : vulkanContext.swapChainImageViews)
        {
            vkDestroyImageView(vulkanContext.virtualGPU, imageView, nullptr);
        }
        vkDestroySwapchainKHR(vulkanContext.virtualGPU, vulkanContext.swapChain, nullptr);
        vkDestroyDevice(vulkanContext.virtualGPU, nullptr);
#ifdef CHIN_VK_VAL_LAYERS
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(vulkanContext.instance, vulkanContext.debugMessenger, nullptr);
        } else {
            CHIN_LOG_ERROR("Failed to destroy Vulkan debug messenger!");
        }
#endif
        vkDestroySurfaceKHR(vulkanContext.instance, vulkanContext.renderSurface, nullptr);
        vkDestroyInstance(vulkanContext.instance, nullptr);
    }

    void CreateSurface(Window::Frame &frame)
    {
        if (glfwCreateWindowSurface(frame.vulkanContext.instance, frame.window, nullptr, &frame.vulkanContext.renderSurface) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL("Failed to create rendering-surface on window: {}", frame.frameSpec.title);
            assert(false);
        }
    }

    void CreateSwapChain(Window::Frame &frame)
    {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(frame.vulkanContext);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats, frame.graphicsSettings);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes, frame.graphicsSettings);
        VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities, frame, frame.graphicsSettings);

        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
        {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = frame.vulkanContext.renderSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(frame.vulkanContext);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentationFamily.value()};
        if (indices.graphicsFamily != indices.presentationFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
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

        if (vkCreateSwapchainKHR(frame.vulkanContext.virtualGPU, &createInfo, nullptr, &frame.vulkanContext.swapChain) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL("Failed to create Vulkan swapchain!");
            assert(false);
        }

        vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount, nullptr);
        frame.vulkanContext.swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount, frame.vulkanContext.swapChainImages.data());

        frame.vulkanContext.swapChainImageFormat = surfaceFormat.format;
        frame.vulkanContext.swapChainExtent = extent;
    }

    void CreateImageViews(Window::Frame &frame)
    {
        frame.vulkanContext.swapChainImageViews.resize(frame.vulkanContext.swapChainImages.size());

        for (size_t i = 0; i < frame.vulkanContext.swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = frame.vulkanContext.swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = frame.vulkanContext.swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(frame.vulkanContext.virtualGPU, &createInfo, nullptr, &frame.vulkanContext.swapChainImageViews[i]) != VK_SUCCESS)
            {
                CHIN_LOG_CRITICAL("Failed to create Vulkan image view!");
                assert(false);
            }
        }
    }

    //TODO: Let user decide and make sure to make the right choice on handhelds like SteamDeck
    void PickPhysicalGPU(VulkanContext& vulkanContext)
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            CHIN_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, devices.data());

        uint32_t extensionCount;
        bool extensionsSupported;
        bool swapChainAdequate;
        for (const auto &device: devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(vulkanContext.deviceExtensions.begin(), vulkanContext.deviceExtensions.end());
            for (const auto& extension : availableExtensions)
            {
                requiredExtensions.erase(extension.extensionName);
            }

            VulkanContext temp;
            temp.physicalGPU = device;
            temp.renderSurface = vulkanContext.renderSurface;
            QueueFamilyIndices queues = findQueueFamilies(temp);

            if ((extensionsSupported = requiredExtensions.empty()))
            {
                SwapChainSupportDetails swapChainDetails = querySwapChainSupport(temp);
                swapChainAdequate = (!swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty());
            }

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU // Arbitrary
                && deviceFeatures.geometryShader // Arbitrary
                && (queues.graphicsFamily == queues.presentationFamily) // Arbitrary
                && queues.isComplete() // Necessary
                && extensionsSupported // Necessary
                && swapChainAdequate)  // Necessary
            {
                vulkanContext.physicalGPU = device;
                return;
            }
        }
        //TODO: System to choose the next best fallback when preferred properties are not met, instead of giving up
        CHIN_LOG_CRITICAL("Failed to choose a GPU with given requirements!");
        assert(false);
    }

    void CreateVirtualGPU(VulkanContext& vulkanContext)
    {
        QueueFamilyIndices indices = findQueueFamilies(vulkanContext);

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

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanContext.deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = vulkanContext.deviceExtensions.data();

        if (vkCreateDevice(vulkanContext.physicalGPU, &deviceCreateInfo, nullptr, &vulkanContext.virtualGPU) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL("Failed to create a Vulkan virtual GPU!");
            assert(false);
        }

        vkGetDeviceQueue(vulkanContext.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanContext.graphicsQueue);
    }

}
