#include "VulkanFunctions.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../ops/Logging.h"
#include "../Window.h"

namespace Chinstrap::Vulkan
{
    void Init(Window::Frame &frame)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        CHIN_LOG_INFO("{} extensions supported!", extensionCount);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = frame.frameSpec.title.c_str();
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

        if (vkCreateInstance(&createInfo, nullptr, &frame.vulkanInstance) != VK_SUCCESS)
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
        for (const char *layerName: frame.validationLayers)
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
        createInfo.enabledLayerCount = static_cast<uint32_t>(frame.validationLayers.size());
        createInfo.ppEnabledLayerNames = frame.validationLayers.data();
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
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(frame.vulkanInstance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(frame.vulkanInstance, &createMessengerInfo, nullptr, &frame.vulkanDebugMessenger);
        } else {
            CHIN_LOG_ERROR("Failed to set up Vulkan debug messenger!");
        }
#endif
    }

    void Shutdown(Window::Frame& frame)
    {
#ifdef CHIN_VK_VAL_LAYERS
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(frame.vulkanInstance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(frame.vulkanInstance, frame.vulkanDebugMessenger, nullptr);
        } else {
            CHIN_LOG_ERROR("Failed to destroy Vulkan debug messenger!");
        }
#endif
        vkDestroyInstance(frame.vulkanInstance, nullptr);
    }
}
