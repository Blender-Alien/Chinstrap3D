#include <Chinstrap.h>

#include "TestGLScene.h"

int main()
{
    Chinstrap::Window::FrameSpec spec("Sandbox", 1920, 1080, true, true);
    Chinstrap::Application::Init("Sandbox Application", spec);

    CHIN_LOG_INFO("We're loggin!");
    CHIN_LOG_WARN("Loggin is addictive!");
    CHIN_LOG_ERROR("I can't stop loggin!");
    CHIN_LOG_CRITICAL("Loggin has consumed me!!");

    Chinstrap::Application::PushScene<Game::TestGLScene>();
    Chinstrap::Application::Run();
}