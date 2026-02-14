#pragma once
#include "../ops/Logging.h"

#include <fstream>

namespace Chinstrap::ChinVulkan { struct VulkanContext; }

namespace Chinstrap::Renderer
{
    void Shutdown(const ChinVulkan::VulkanContext &context);

    void Setup();
    void DrawFrame();
}

static std::vector<char> readFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        CHIN_LOG_ERROR("Failed to open file {}!", filePath);
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
