#include "chinstrap/src/Window.h"
#include "chinstrap/src/Application.h"

#include "TestGLScene.h"
#include "TestGuiScene.h"

int main()
{
    Chinstrap::Application::App app(2); // Create a single app object on the stack

    Chinstrap::Window::FrameSpec frameSpec;
    Chinstrap::Window::ViewPortSpec viewportSpec;
    frameSpec.Create("Sandbox", 1920, 1080);
    viewportSpec.Create(0.0f, 0.0f, 1.0f, 1.0f);

    // Initialize the stack object once
    if (app.Init("Sandbox Application",
        frameSpec,
        viewportSpec)
    != 0)
    {
        return -1;
    }

    /* Choose which scenes you want on the stack from the start,
     * ORDER matters here. Scenes will be updated and rendered
     * in the order that they are pushed */
    app.PushScene<Game::TestGLScene>();
    app.PushScene<Game::TestGUIScene>();

    // Start running your application
    app.Run();
}
