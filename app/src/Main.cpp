#include "src/Application.h"
#include <Chinstrap.h>

int main()
{
    Chinstrap::Window::FrameSpec spec("Sandbox", 1920, 1080, true, true);

    Chinstrap::Application::Init("Sandbox Application", spec);
    Chinstrap::Application::Run();
}
