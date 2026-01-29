#pragma once

#include "chinstrap/src/Scene.h"

namespace Game
{
    struct TestGLScene : public Chinstrap::Scene
    {
        void OnBegin() override;
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;
    };
}
