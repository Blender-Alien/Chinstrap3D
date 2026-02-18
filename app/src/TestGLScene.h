#pragma once

#include "chinstrap/src/Scene.h"

namespace Game
{
    struct TestGLScene : public Chinstrap::Scene
    {
        void OnBegin() override;
        void OnShutdown() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;

        [[nodiscard]] std::string GetName() const override { return "TestGLScene"; }
    };
}
