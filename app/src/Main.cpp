#if CHIN_DEBUG
#include "spdlog/spdlog.h"
#endif

#include <Chinstrap.h>

#include "TestGLScene.h"

int main()
{
    Chinstrap::Window::FrameSpec spec("Sandbox", 1920, 1080, true, true);
    Chinstrap::Application::Init("Sandbox Application", spec);

#if CHIN_DEBUG
    spdlog::info("We're loggin!");
#endif


    Chinstrap::Application::PushScene<Game::TestGLScene>();
    Chinstrap::Application::Run();
}
