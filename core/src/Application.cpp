#include "Application.h"

#include "glad.h"
#include "GLFW/glfw3.h"

#include <assert.h>

namespace Chinstrap
{
    namespace Application
    {

        namespace // access only in this file
        {
            static App* appInstance = nullptr;
        }

        App::App()
        {
            assert(appInstance == nullptr);
        }

        App::~App()
        {
            glfwTerminate();
            appInstance = nullptr;
        }

        App& App::Get()
        {
            assert(appInstance);
            return *appInstance;
        }

        int Init(const std::string& appName)
        {
            appInstance = new App();
            appInstance->name = appName;

            if (!glfwInit())
            {
                return -1;
            }

            appInstance->window = glfwCreateWindow(1920, 1080, appInstance->name.c_str(), NULL, NULL);
            if (!appInstance->window)
            {
                glfwTerminate();
                return -1;
            }
            
            glfwMakeContextCurrent(appInstance->window);

            if (gladLoadGL() == 0)
            {
                glfwTerminate();
                return -1;
            }

            return 0;
        }

        void Run()
        {
            appInstance->running = true;

            while (appInstance->running)
            {
                if (glfwWindowShouldClose(appInstance->window))
                {
                    appInstance->running = false;
                }

                glClear(GL_COLOR_BUFFER_BIT);
                glfwSwapBuffers(appInstance->window);
                glfwPollEvents();
            }

            Stop();
        }

        void Stop()
        {
            appInstance->running = false;
        }
    }
     

}



int Chinstrap::glfwTest()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Load and check glad after valid context established
    if (gladLoadGL() == 0)
    {
        glfwTerminate();
        return -1;
    }

//  /* Loop until the user closes the window */
//  while (!glfwWindowShouldClose(window))
//  {
//      /* Render here */
//      glClear(GL_COLOR_BUFFER_BIT);
//
//      /* Swap front and back buffers */
//      glfwSwapBuffers(window);
//
//      /* Poll for and process events */
//      glfwPollEvents();
//  }

    glfwTerminate();
    return 0;
}
