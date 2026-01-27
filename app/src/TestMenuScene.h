#pragma once

#include <Chinstrap.h>

namespace Game
{
    struct TestMenuScene : public Chinstrap::Scene
    {
        void OnBegin() override;
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;
    };
}
