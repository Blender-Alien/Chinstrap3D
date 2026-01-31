#include "RendererData.h"

#include "../ops/Logging.h"

namespace Chinstrap
{
    namespace RendererData
    {
        Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
            : vertexPath(vertexPath), fragmentPath(fragmentPath)
        {
        }
    }
}
