#pragma once

#include "src/InputEvents.h"
#include <Chinstrap.h>

namespace Game
{
    struct TestMenuScene : public Chinstrap::Scene
    {
        void OnBegin() override;
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(Chinstrap::KeyPressedEvent &event);
    };
}
