#pragma once
#include <string>
#include <unordered_map>

namespace Chinstrap
{
    struct Shader
    {
        const std::string filepath;
        unsigned int rendererID;
        std::unordered_map<std::string, int> uniformLocationCache;

        explicit Shader(const std::string& filepath);
        ~Shader();
    };

}
