#include "Renderer.h"

#include <iostream>

#include "glad.h"
#include "Logging.h"

namespace Chinstrap
{
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
