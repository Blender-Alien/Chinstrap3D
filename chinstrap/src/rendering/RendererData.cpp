#include "RendererData.h"

#include "../serialization/SerializeReadable.h"

#include "../Application.h"
#include "../ops/Logging.h"

bool Chinstrap::Renderer::CreateShader(const ChinVulkan::VulkanContext &vulkanContext, VkShaderModule& shaderModule, const char* codeBegin, const char* codeEnd)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeEnd - codeBegin;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(codeBegin);

    CHIN_LOG_INFO_VULKAN_F("Creating shader module with code size {}", codeEnd - codeBegin);

    if (vkCreateShaderModule(vulkanContext.virtualGPU, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        CHIN_LOG_ERROR("Failed to create shader module!");
        return false;
    }
    return true;
}

bool Chinstrap::Renderer::ShaderLoader(VkShaderModule& shaderModule, std::string_view OSFilePath)
{
    std::ifstream file(OSFilePath.data(), std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        CHIN_LOG_ERROR("Shader loader failed to open file {}!", OSFilePath);
        return false;
    }

    size_t fileSize = file.tellg();
    char buffer[fileSize];

    file.seekg(0);
    file.read(buffer, fileSize);
    file.close();

    return CreateShader(Application::App::GetVulkanContext(), shaderModule,&buffer[0], &buffer[fileSize]);
}

Chinstrap::Renderer::Material::Material(const Memory::FilePath& vertexShaderPath_arg, const Memory::FilePath& fragmentShaderPath_arg)
    : vertexShaderPath(vertexShaderPath_arg), fragmentShaderPath(fragmentShaderPath_arg)
{
}

Chinstrap::Renderer::Material::~Material()
{
    vkDestroyPipeline(pVulkanContext->virtualGPU, pipeline, nullptr);
    vkDestroyPipelineLayout(pVulkanContext->virtualGPU, pipelineLayout, nullptr);
    CHIN_LOG_INFO_VULKAN("Destroyed Material");
}

bool Chinstrap::Renderer::MaterialLoader(Material* dataPtr, std::string_view OSFilePath)
{
    std::ifstream fileStream(OSFilePath.data());
    if (!fileStream.is_open())
    {
        return false;
    }

    std::string fieldData[2];
    uint32_t index = 0;

    std::string line;
    while (std::getline(fileStream, line))
    {
        if (Serialization::IsLineField(line))
        {
            fieldData[index] = std::move(line);
        }
        ++index;
    }

    Memory::FilePath vertexShaderPath;
    {
        auto positions = Serialization::GetFieldContent(fieldData[0]);
        vertexShaderPath.Hash(fieldData[0].substr(std::get<0>(positions), std::get<1>(positions)));
    }
    Memory::FilePath fragmentShaderPath;
    {
        auto positions = Serialization::GetFieldContent(fieldData[1]);
        fragmentShaderPath.Hash(fieldData[1].substr(std::get<0>(positions), std::get<1>(positions)));
    }

    auto* material = new(dataPtr) Material(vertexShaderPath, fragmentShaderPath);


    // Temporary
    {
        auto positions = Serialization::GetFieldContent(fieldData[0]);
        material->vertexPath = fieldData[0].substr(std::get<0>(positions), std::get<1>(positions));
    }
    {
        auto positions = Serialization::GetFieldContent(fieldData[1]);
        material->fragmentPath = fieldData[1].substr(std::get<0>(positions), std::get<1>(positions));
    }
    material->Create(&Application::App::GetVulkanContext());

    return true;
}

bool Chinstrap::Renderer::Material::Create(ChinVulkan::VulkanContext* vulkanContext)
{
    pVulkanContext = vulkanContext;

    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
    // TODO: These won't return anything because the hashID's were never interned into filePathMap
    //       We need a general String hashmap, but let's just directly pass the strings for now
    /*
    auto vertexPath = Application::App::GetFilePathMap().Lookup(vertexShaderPath);
    auto fragmentPath = Application::App::GetFilePathMap().Lookup(fragmentShaderPath);
    if (!vertexPath.has_value() || !fragmentPath.has_value())
    {
        return false;
    }
    */

    if (!ShaderLoader(vertexShader, Memory::FilePath::ConvertToOSPath(vertexPath).get()))
    {
        return false;
    }
    if (!ShaderLoader(fragmentShader, Memory::FilePath::ConvertToOSPath(fragmentPath).get()))
    {
        return false;
    }

    ExampleCreateMaterial(vertexShader, fragmentShader);

    vkDestroyShaderModule(Application::App::GetVulkanContext().virtualGPU, vertexShader, nullptr);
    vkDestroyShaderModule(Application::App::GetVulkanContext().virtualGPU, fragmentShader, nullptr);

    return true;
}

VkVertexInputBindingDescription Chinstrap::Renderer::GetVertexBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Chinstrap::Renderer::GetVertexAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

void Chinstrap::Renderer::Material::ExampleCreateMaterial(VkShaderModule vertexShader, VkShaderModule fragmentShader)
{

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertexShader;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragmentShader;
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

    auto bindingDescription = GetVertexBindingDescription();
    auto attributeDescriptions = GetVertexAttributeDescriptions();

    // Hard coding vertex info in vertex shader for now
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};

    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pVulkanContext->swapChainExtent.width);
    viewport.height = static_cast<float>(pVulkanContext->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = pVulkanContext->swapChainExtent;

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

    if (vkCreatePipelineLayout(pVulkanContext->virtualGPU, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create pipeline layout!");
        assert(false);
    }

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.pNext = VK_NULL_HANDLE;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &pVulkanContext->swapChainImageFormat;

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
    graphicsPipelineCreateInfo.layout = pipelineLayout;

    if (vkCreateGraphicsPipelines(pVulkanContext->virtualGPU, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &pipeline)
        != VK_SUCCESS)
    {
        CHIN_LOG_CRITICAL_VULKAN("Failed to create graphics pipeline!");
        assert(false);
    }

    CHIN_LOG_INFO_VULKAN("Successfully created barebones material");
}
