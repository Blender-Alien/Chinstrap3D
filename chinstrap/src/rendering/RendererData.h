#pragma once
#include <string>
#include <unordered_map>

namespace Chinstrap
{
    namespace RendererData
    {
        struct Shader
        {
            std::string vertexPath;
            std::string fragmentPath;
            unsigned int ID;

            explicit Shader(const std::string& vertexPath, const std::string& fragmentPath);
        };

    }
}
