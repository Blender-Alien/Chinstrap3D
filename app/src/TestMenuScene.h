#pragma once

#include <Chinstrap.h>

namespace Game
{
    struct TestMenuScene : public Chinstrap::Scene
    {
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(Chinstrap::Event &event);
    };
}
