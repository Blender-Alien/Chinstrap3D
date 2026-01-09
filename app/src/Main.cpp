#include <iostream>
#include "Application.h"

int main()
{
    Shader shader;
    Shader shader2 = Shading::Bind(shader);
    std::cout << shader2.m_Filepath << "\n";
    std::cout << "Hello World!" << std::endl; 
}
