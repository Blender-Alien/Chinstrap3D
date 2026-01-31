#include "Renderer.h"

#include "glad.h"
#include "../ops/Logging.h"

#include <fstream>
#include <sstream>

namespace Chinstrap
{
    namespace Renderer
    {
        void BindShader(const RendererData::Shader &shader)
        {
            glUseProgram(shader.ID);
        }

        void UnbindShader()
        {
            glUseProgram(0);
        }

        void setUniformBool(RendererData::Shader& shader, const std::string &name, bool value)
        {
            GLCall(glUniform1i(glGetUniformLocation(shader.ID, name.c_str()), static_cast<int>(value)));
        }
        void setUniformInt(RendererData::Shader& shader, const std::string &name, int value)
        {
            GLCall(glUniform1i(glGetUniformLocation(shader.ID, name.c_str()), value));
        }
        void setUniformFloat(RendererData::Shader& shader, const std::string &name, float value)
        {
            GLCall(glUniform1f(glGetUniformLocation(shader.ID, name.c_str()), value));
        }

        void CompileShader(RendererData::Shader &shader)
        {
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;

            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                vShaderFile.open(shader.vertexPath);
                fShaderFile.open(shader.fragmentPath);
                std::stringstream vShaderStream , fShaderStream;

                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();

                vShaderFile.close();
                fShaderFile.close();

                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch (std::ifstream::failure e)
            {
                CHIN_LOG_ERROR("Shader could not be read! {0} and {1}", shader.vertexPath, shader.fragmentPath);
            }
            unsigned int vertex, fragment;
            int success;
            char infoLog[512];

            vertex = glCreateShader(GL_VERTEX_SHADER);
            const char* shaderCode = vertexCode.c_str();
            glShaderSource(vertex, 1, &shaderCode, nullptr);
            glCompileShader(vertex);
            glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
                CHIN_LOG_ERROR("VertexShader compilation failed! {0}", infoLog);
            }

            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            const char* shaderCode2 = fragmentCode.c_str();
            glShaderSource(fragment, 1, &shaderCode2, nullptr);
            glCompileShader(fragment);
            glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
                CHIN_LOG_ERROR("FragmentShader compilation failed! {0}", infoLog);
            }

            shader.ID = glCreateProgram();
            glAttachShader(shader.ID, vertex);
            glAttachShader(shader.ID, fragment);
            glLinkProgram(shader.ID);
            glGetProgramiv(shader.ID, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader.ID, 512, nullptr, infoLog);
                CHIN_LOG_ERROR("Shader linking failed! {0}", infoLog);
            }

            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }
    }

    namespace GLDebug
    {
        void GLClearError()
        {
            while (glGetError() != GL_NO_ERROR);
        }

        bool GLLogCall(const char *function, const char *file, int line)
        {
            while (GLenum error = glGetError())
            {
                CHIN_LOG_ERROR("[OpenGL Error] (0x{0:x}):\n{1}\nin file: {2} line:{3}", error, function, file, line);
                return false;
            }
            return true;
        }
    }
}
