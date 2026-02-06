#include "VulkanFunctions.h"

#define GLFW_INCLUDE_VULKAN
#include <set>

#include "GLFW/glfw3.h"

#include "../ops/Logging.h"
#include "../Window.h"
#include "VulkanData.h"

namespace Chinstrap::ChinVulkan
{
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

    //TODO: Let user decide and make sure to make the right choice on handhelds like SteamDeck
    void PickPhysicalGPU(VulkanContext& vulkanContext)
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            CHIN_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, devices.data());

        for (const auto &device: devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            VulkanContext temp;
            temp.physicalGPU = device;
            temp.renderSurface = vulkanContext.renderSurface;
            QueueFamilyIndices queues = findQueueFamilies(temp);

            // These requirements are arbitrary for now
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader
                && queues.isComplete() && (queues.graphicsFamily == queues.presentationFamily))
            {
                vulkanContext.physicalGPU = device;
                return;
            }
        }
        //TODO: System to choose the next best fallback when preferred properties are not met, instead of giving up
        CHIN_LOG_CRITICAL("Failed to choose a GPU with given requirements!");
        assert(false);
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

        if (vkCreateDevice(vulkanContext.physicalGPU, &deviceCreateInfo, nullptr, &vulkanContext.virtualGPU) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL("Failed to create a Vulkan virtual GPU!");
            assert(false);
        }

        vkGetDeviceQueue(vulkanContext.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanContext.graphicsQueue);
    }
}
