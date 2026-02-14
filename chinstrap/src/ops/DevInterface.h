#pragma once

namespace Chinstrap::DevInterface
{
    void Initialize(float fontSize = 13.0f);
    void Render();
    void Render(void(*lambda)());
    void Shutdown();

    void ContextInfo(float posScaleX, float posScaleY);
    void PerformanceInfo(float posScaleX, float posScaleY);
}
