#pragma once

#include <string>
#include <vector>
#include <memory>


namespace Chinstrap
{
    // Forward declerations
    class Scene;

    // Only one application will exist at time, thus no class or struct is used
    namespace Application
    {
        extern std::string Name;

        extern std::vector<std::unique_ptr<Scene>> SceneStack;

        void Run();
    }

    int glfwTest();
}

