#pragma once

#include "RendererData.h"

#ifdef CHIN_DEBUG
#define GLCall(x) Chinstrap::GLDebug::GLClearError();\
    x;\
    Chinstrap::GLDebug::GLLogCall(#x, __FILE__, __LINE__)
#else
#define GLCall(x) x;\

#endif

namespace Chinstrap
{
    namespace Renderer
    {
        void BindShader(const RendererData::Shader& shader);
        void UnbindShader();

        //TODO: Multithreaded shader-compile handler
        void CompileShader(RendererData::Shader& shader);

        void setUniformBool(RendererData::Shader& shader, const std::string &name, bool value);
        void setUniformInt(RendererData::Shader& shader, const std::string &name, int value);
        void setUniformFloat(RendererData::Shader& shader, const std::string &name, float value);
    }

    namespace GLDebug
    {
        void GLClearError();
        bool GLLogCall(const char* function, const char* file, int line);
    }
}
