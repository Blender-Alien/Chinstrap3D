#include "RendererData.h"
#include "../ops/Logging.h"

Chinstrap::Renderer::Material::Material(const ChinVulkan::VulkanContext &vulkanContext,
                   const std::vector<char> &vertexShaderCode,
                   const std::vector<char> &fragmentShaderCode)
    : vulkanContext(vulkanContext)
{
    /* Test configuration */
    ExampleCreateMaterial(vulkanContext, *this, vertexShaderCode, fragmentShaderCode);
}

void Chinstrap::Renderer::Material::Cleanup()
{
    vkDestroyPipeline(vulkanContext.virtualGPU, pipeline, nullptr);
    vkDestroyPipelineLayout(vulkanContext.virtualGPU, pipelineLayout, nullptr);
    CHIN_LOG_INFO_VULKAN("Destroyed Material and resources");
}

// TODO: Handle shaders properly
inline VkShaderModule CreateShaderModule(const Chinstrap::ChinVulkan::VulkanContext &vulkanContext, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vulkanContext.virtualGPU, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR("Failed to create shader module!");
    }
    return shaderModule;
}

void Chinstrap::Renderer::ExampleCreateMaterial(const ChinVulkan::VulkanContext &vulkanContext, Material &material, const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode)
{
    CHIN_LOG_INFO_VULKAN_F("VertexShader size: {0}; FragmentShader size: {1}", vertexCode.size(),
                           fragmentCode.size());

    VkShaderModule vertShaderModule = CreateShaderModule(vulkanContext, vertexCode);
    VkShaderModule fragShaderModule = CreateShaderModule(vulkanContext, fragmentCode);

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

    //auto bindingDescription = Renderer::Vertex::GetBindingDescription();
    //auto attributeDescriptions = Renderer::Vertex::GetAttributeDescriptions();

    // Hard coding vertex info in vertex shader for now, thus no input
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    /*
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    */
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
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
