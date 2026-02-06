#include "VulkanFunctions.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../ops/Logging.h"
#include "../Window.h"
#include "VulkanData.h"

namespace Chinstrap::ChinVulkan
{
    void Init(VulkanSetupData &vulkanData, const std::string& name)
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

        if (vkCreateInstance(&createInfo, nullptr, &vulkanData.instance) != VK_SUCCESS)
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
        for (const char *layerName: vulkanData.validationLayers)
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
        createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanData.validationLayers.size());
        createInfo.ppEnabledLayerNames = vulkanData.validationLayers.data();
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
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanData.instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(vulkanData.instance, &createMessengerInfo, nullptr, &vulkanData.debugMessenger);
        } else {
            CHIN_LOG_ERROR("Failed to set up Vulkan debug messenger!");
        }
#endif
    }

    void Shutdown(VulkanSetupData& vulkanData)
    {
        vkDestroyDevice(vulkanData.virtualGPU, nullptr);
#ifdef CHIN_VK_VAL_LAYERS
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanData.instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(vulkanData.instance, vulkanData.debugMessenger, nullptr);
        } else {
            CHIN_LOG_ERROR("Failed to destroy Vulkan debug messenger!");
        }
#endif
        vkDestroyInstance(vulkanData.instance, nullptr);
    }

    //TODO: Let user decide and make sure to make the right choice on handhelds like SteamDeck
    void PickPhysicalGPU(VulkanSetupData& data)
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(data.instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            CHIN_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(data.instance, &deviceCount, devices.data());

        for (const auto &device: devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            // These requirements are arbitrary for now
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader
                || findQueueFamilies(device).isComplete())
            {
                data.physicalGPU = device;
                return;
            }
        }
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int index = 0;
        for (const auto &queueFamily: queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = index;
            }
            if (indices.isComplete()) {break;}
            ++index;
        }

        return indices;
    }

    void CreateVirtualGPU(VulkanSetupData& vulkanData)
    {
        QueueFamilyIndices indices = findQueueFamilies(vulkanData.physicalGPU);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        if (vkCreateDevice(vulkanData.physicalGPU, &deviceCreateInfo, nullptr, &vulkanData.virtualGPU) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL("Failed to create a Vulkan virtual GPU!");
            assert(false);
        }

        vkGetDeviceQueue(vulkanData.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanData.graphicsQueue);
    }
}
