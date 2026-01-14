#pragma once

#include "Window.h"
#include "Layer.h"

#include <string>
#include <vector>
#include <memory>

namespace Chinstrap
{
    namespace Application
    {

        struct AppSpecification
        {
            std::string Name = "Application";
            WindowSpecification WindowSpec;
        };

        struct App
        {
            App();
            ~App();

            AppSpecification m_Specification;
            
            std::vector<std::unique_ptr<Layer>> m_LayerStack;

        };
    
        void Run(const App&);

    };

    int glfwTest();
}

