#pragma once

#include <Chinstrap.h>

namespace Game
{
    struct TestGUIScene : public Chinstrap::Scene
    {
        ~TestGUIScene() override;
        void OnBegin() override;
        void OnUpdate() override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;
    };
}
