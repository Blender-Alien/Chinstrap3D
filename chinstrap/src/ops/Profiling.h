#pragma once
#include "GLFW/glfw3.h"

#define CHIN_PROFILE_TIME(func, resultTarget) { Chinstrap::Profile::Timer* timer = new Chinstrap::Profile::Timer;\
        func;\
        resultTarget = timer->getTime();\
        delete timer; }

namespace Chinstrap::Profile
{
        struct Timer
        {
                double startTime = glfwGetTime();

                [[nodiscard]] float getTime() const
                {
                        return static_cast<float>(glfwGetTime() - startTime);
                }
        };
}
