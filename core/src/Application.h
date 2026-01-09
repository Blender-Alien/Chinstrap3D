#pragma once
#include <string>

struct Shader
{
    std::string m_Filepath;

};

namespace Shading
{
    Shader Bind(Shader shader);
}

