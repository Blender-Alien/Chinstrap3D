#pragma once

#include <memory>
#include <functional>

namespace Chinstrap::Window {struct Frame;}

namespace Chinstrap::DevInterface
{
    void Initialize(float fontSize = 13.0f);
    void Render();
    void Render(std::function<void()> lambda);
    void Shutdown();

    void ContextInfo();
}