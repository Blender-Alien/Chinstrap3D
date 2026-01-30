#pragma once

#include "chinstrap/src/Scene.h"

namespace Game
{
    struct TestGUIScene : public Chinstrap::Scene
    {
        ~TestGUIScene() override;
        void OnBegin() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;

        [[nodiscard]] std::string GetName() const override { return "TestGuiScene"; }
    };
}
