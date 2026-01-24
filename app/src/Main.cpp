#include <Chinstrap.h>

#include "TestGLScene.h"

int main()
{
    Chinstrap::Window::FrameSpec frame("Sandbox", 2560, 1440, true, true);
    Chinstrap::Window::ViewPortSpec viewport(100, 100, 1920, 1080);
    Chinstrap::Application::Init("Sandbox Application", frame, viewport);
    Chinstrap::Application::PushScene<Game::TestGLScene>();
    Chinstrap::Application::Run();
}