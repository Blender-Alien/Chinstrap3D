#include "VulkanFunctions.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../ops/Logging.h"
#include "../Window.h"
#include "VulkanData.h"
#include "Renderer.h"

#include <set>
#include <limits>
#include <algorithm>
#include <any>

namespace Chinstrap::ChinVulkan
{
    namespace //* Externally Unavailable Functions *//
    {
        SwapChainSupportDetails querySwapChainSupport(const VulkanContext& vulkanContext)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanContext.physicalGPU, vulkanContext.windowSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalGPU, vulkanContext.windowSurface, &formatCount, nullptr);
            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalGPU, vulkanContext.windowSurface, &formatCount, details.formats.data());
            }

            uint32_t presentationModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext.physicalGPU, vulkanContext.windowSurface, &presentationModeCount, nullptr);
            if (presentationModeCount != 0)
            {
                details.presentModes.resize(presentationModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext.physicalGPU, vulkanContext.windowSurface, &presentationModeCount, details.presentModes.data());
            }

            return details;
        }

        QueueFamilyIndices findQueueFamilies(const VulkanContext& vulkanContext)
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
                vkGetPhysicalDeviceSurfaceSupportKHR(vulkanContext.physicalGPU, index, vulkanContext.windowSurface, &presentationSupport);
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
            if (settings.colorSpace != UserSettings::ColorSpaceMode::SRGB)
            {
                CHIN_LOG_ERROR_VULKAN("HDR is not supported!");
                settings.colorSpace = UserSettings::ColorSpaceMode::SRGB; // There was no available HDR format
            }
            CHIN_LOG_ERROR_VULKAN("Chose fallback format and colorspace!");
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
                CHIN_LOG_ERROR_VULKAN("Selected VSync Mode is not supported!");
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
            CHIN_LOG_CRITICAL_VULKAN("Failed to create instance!!!");
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
            CHIN_LOG_ERROR_VULKAN("Failed to set up Vulkan debug messenger!");
        }
#endif
        CHIN_LOG_INFO_VULKAN("Successfully initialized");
    }

    void CreateSurface(Window::Frame &frame)
    {
        if (glfwCreateWindowSurface(frame.vulkanContext.instance, frame.window, nullptr, &frame.vulkanContext.windowSurface) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN_F("Failed to create rendering-surface on window: {}", frame.frameSpec.title);
            assert(false);
        }
        CHIN_LOG_INFO_VULKAN("Successfully created rendering surface");
    }

    void CreateSwapChain(Window::Frame &frame)
    {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(frame.vulkanContext);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats, frame.graphicsSettings);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes, frame.graphicsSettings);
        VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities, frame, frame.graphicsSettings);

        CHIN_LOG_INFO_VULKAN_F("SwapChain Info: Res: {0}x{1}", extent.width, extent.height);

        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
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
            CHIN_LOG_CRITICAL_VULKAN("Failed to create SwapChain!");
            assert(false);
        }

        vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount, nullptr);
        frame.vulkanContext.swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(frame.vulkanContext.virtualGPU, frame.vulkanContext.swapChain, &imageCount, frame.vulkanContext.swapChainImages.data());

        frame.vulkanContext.swapChainImageFormat = surfaceFormat.format;
        frame.vulkanContext.swapChainExtent = extent;
        CHIN_LOG_INFO_VULKAN("Successfully created SwapChain");
    }

    void CreateImageViews(const VulkanContext &vulkanContext, std::vector<VkImageView> &imageViews)
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

    //TODO: Let user decide and make sure to make the right choice on handhelds like SteamDeck
    void PickPhysicalGPU(VulkanContext& vulkanContext)
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            CHIN_LOG_CRITICAL_VULKAN("Failed to find GPUs with Vulkan support!");

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
            temp.windowSurface = vulkanContext.windowSurface;
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
                CHIN_LOG_INFO_VULKAN("Successfully chose GPU");
                return;
            }
        }
        //TODO: System to choose the next best fallback when preferred properties are not met, instead of giving up
        CHIN_LOG_CRITICAL_VULKAN("Failed to choose a GPU with given requirements!");
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
            CHIN_LOG_CRITICAL_VULKAN("Failed to create a Vulkan virtual GPU!");
            assert(false);
        }

        vkGetDeviceQueue(vulkanContext.virtualGPU, indices.graphicsFamily.value(), 0, &vulkanContext.graphicsQueue);
        CHIN_LOG_INFO_VULKAN("Successfully created virtual GPU");
    }

    void CreateKitchen(const VulkanContext &vulkanContext, Kitchen &kitchen, const VkRenderPass& renderPass)
    {
        CHIN_LOG_INFO_VULKAN_F("FragShader size: {0}; VertShader size: {1}", kitchen.fragmentShaderCode.size(), kitchen.vertexShaderCode.size());

        VkShaderModule vertShaderModule = CreateShaderModule(vulkanContext, kitchen.vertexShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(vulkanContext, kitchen.fragmentShaderCode);

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
        scissor.offset = { 0, 0 };
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

        // Disabled for now
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        if (vkCreatePipelineLayout(vulkanContext.virtualGPU, &pipelineLayoutCreateInfo, nullptr, &kitchen.pipelineLayout) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create pipeline layout!");
            assert(false);
        }

        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.stageCount = 2;
        graphicsPipelineCreateInfo.pStages = shaderStages;
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        graphicsPipelineCreateInfo.layout = kitchen.pipelineLayout;

        graphicsPipelineCreateInfo.renderPass = renderPass;
        graphicsPipelineCreateInfo.subpass = 0;

        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(vulkanContext.virtualGPU, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &kitchen.pipeline)
            != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics pipeline!");
            assert(false);
        }

        CHIN_LOG_INFO_VULKAN("Successfully created graphics pipeline");

        vkDestroyShaderModule(vulkanContext.virtualGPU, vertShaderModule, nullptr);
        vkDestroyShaderModule(vulkanContext.virtualGPU, fragShaderModule, nullptr);
    }

    void CreateRenderPass(const VulkanContext &vulkanContext, VkRenderPass& renderPass)
    {

        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.format = vulkanContext.swapChainImageFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachmentReference = {};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &attachmentReference;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        if (vkCreateRenderPass(vulkanContext.virtualGPU, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create render pass!");
            assert(false);
        }
        CHIN_LOG_INFO_VULKAN("Successfully created render pass");
    }

    void CreateFramebuffers(const VulkanContext &vulkanContext, std::vector<VkFramebuffer>& framebuffers, const std::vector<VkImageView>& imageViews, const VkRenderPass& renderPass)
    {
        framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); ++i)
        {
            const VkImageView attachments[] = { imageViews[i] };

            VkFramebufferCreateInfo framebufferCreateInfo = {};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.width = vulkanContext.swapChainExtent.width;
            framebufferCreateInfo.height = vulkanContext.swapChainExtent.height;
            framebufferCreateInfo.layers = 1;

            if (vkCreateFramebuffer(vulkanContext.virtualGPU, &framebufferCreateInfo, nullptr, &framebuffers[i])
                != VK_SUCCESS)
            {
                CHIN_LOG_ERROR_VULKAN("Failed to create framebuffer!");
            }
        }
    }

    void CreateCommandPool(const VulkanContext &vulkanContext, VkCommandPool& commandPool)
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkanContext);

        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(vulkanContext.virtualGPU, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command pool!");
        }
    }

    void CreateCommandBuffer(const VulkanContext &vulkanContext, VkCommandBuffer& commandBuffer, const VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(vulkanContext.virtualGPU, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics command buffer!");
        }
    }

    void TestRecordCommandBuffer(VkCommandBuffer &targetCommandBuffer, const Restaurant& restaurant, const Kitchen& kitchen, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(targetCommandBuffer, &beginInfo) != VK_SUCCESS)
        {
            CHIN_LOG_ERROR_VULKAN("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = restaurant.renderPass;
        renderPassBeginInfo.framebuffer = restaurant.swapChainFramebuffers[imageIndex];
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = restaurant.vulkanContext.swapChainExtent;

        constexpr VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassBeginInfo.pClearValues = &clearValue;
        renderPassBeginInfo.clearValueCount = 1;

        vkCmdBeginRenderPass(targetCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(targetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, kitchen.pipeline);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(restaurant.vulkanContext.swapChainExtent.width);
        viewport.height = static_cast<float>(restaurant.vulkanContext.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(targetCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = restaurant.vulkanContext.swapChainExtent;
        vkCmdSetScissor(targetCommandBuffer, 0, 1, &scissor);

        vkCmdDraw(targetCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(targetCommandBuffer);
        if (vkEndCommandBuffer(targetCommandBuffer) != VK_SUCCESS)
        {
            CHIN_LOG_ERROR_VULKAN("Failed to end recording command buffer!");
        }
    }

    void CreateSyncObjects(VulkanContext &vulkanContext)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(vulkanContext.virtualGPU, &semaphoreCreateInfo, nullptr, &vulkanContext.imageAvailableSemaphore)
            != VK_SUCCESS ||
            vkCreateSemaphore(vulkanContext.virtualGPU, &semaphoreCreateInfo, nullptr, &vulkanContext.renderFinishedSemaphore)
            != VK_SUCCESS ||
            vkCreateFence(vulkanContext.virtualGPU, &fenceCreateInfo, nullptr, &vulkanContext.inFlightFence)
            != VK_SUCCESS)
        {
            CHIN_LOG_CRITICAL_VULKAN("Failed to create semaphores!");
        }
    }

    void Shutdown(VulkanContext& vulkanContext)
    {
        vkDestroySemaphore(vulkanContext.virtualGPU, vulkanContext.imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(vulkanContext.virtualGPU, vulkanContext.renderFinishedSemaphore, nullptr);
        vkDestroyFence(vulkanContext.virtualGPU, vulkanContext.inFlightFence, nullptr);

        vkDestroySwapchainKHR(vulkanContext.virtualGPU, vulkanContext.swapChain, nullptr);
        vkDestroyDevice(vulkanContext.virtualGPU, nullptr);
#ifdef CHIN_VK_VAL_LAYERS
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vulkanContext.instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
        {
            func(vulkanContext.instance, vulkanContext.debugMessenger, nullptr);
        } else {
            CHIN_LOG_ERROR_VULKAN("Failed to destroy debug messenger!");
        }
#endif
        vkDestroySurfaceKHR(vulkanContext.instance, vulkanContext.windowSurface, nullptr);
        vkDestroyInstance(vulkanContext.instance, nullptr);
        CHIN_LOG_INFO_VULKAN("Successfully shut down");
    }
}
