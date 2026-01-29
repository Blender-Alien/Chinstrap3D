#include "chinstrap/src/Window.h"
#include "chinstrap/src/Application.h"

#include "TestGLScene.h"
#include "TestGuiScene.h"

int main()
{
    Chinstrap::Window::FrameSpec frame("Sandbox", 1920, 1080, true, true);
    Chinstrap::Window::ViewPortSpec viewport(0.0f, 0.0f, 1.0f, 1.0f);

    Chinstrap::Application::Init("Sandbox Application", frame, viewport);
    Chinstrap::Application::PushScene<Game::TestGLScene>();
    Chinstrap::Application::PushScene<Game::TestGUIScene>();

    Chinstrap::Application::Run();
}
