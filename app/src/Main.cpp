#include <Chinstrap.h>

#include "TestGLScene.h"

int main()
{
    Chinstrap::Window::FrameSpec spec("Sandbox", 1920, 1080, true, true);
    Chinstrap::Application::Init("Sandbox Application", spec);

    CHIN_INFO("We're loggin!");

    Chinstrap::Application::PushScene<Game::TestGLScene>();
    Chinstrap::Application::Run();
}
