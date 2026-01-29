#pragma once

#include "RendererData.h"

#ifdef CHIN_DEBUG
#define GLCall(x) Chinstrap::GLDebug::GLClearError();\
    x;\
    Chinstrap::GLDebug::GLLogCall(#x, __FILE__, __LINE__)
#else
#define GLCall(x)

#endif

namespace Chinstrap
{
    namespace Renderer
    {

        //void Draw(const VertexArray& vertexArray, const IndexBuffer& indexBuffer, const Shader& shader);

        void Clear();
    }

    namespace Shading
    {
        void Bind(const Shader& shader);
        void Unbind();
        void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3, Shader& shader);
    }

    namespace GLDebug
    {
        void GLClearError();
        bool GLLogCall(const char* function, const char* file, int line);
    }
}
