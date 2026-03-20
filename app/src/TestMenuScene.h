#pragma once

#include "chinstrap/src/Scene.h"

namespace Game
{
    struct TestMenuScene : public Chinstrap::Scene
    {
        Chinstrap::Renderer::Material* material = nullptr;

        using Scene::Scene;

        void OnBegin() override;
        void OnUpdate(float deltaTime) override;
        void OnRender(uint32_t currentFrame) override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;

        [[nodiscard]] std::string GetName() const override { return "TestMenuScene"; }
    };
}
