#pragma once

#include <Chinstrap.h>

#include "src/InputEvents.h"

namespace Game
{
    struct TestGLScene : public Chinstrap::Scene
    {
        void OnBegin() override;
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(Chinstrap::Event &event);
    };
}
