#pragma once

#include "chinstrap/src/Scene.h"

#include "chinstrap/src/rendering/RendererData.h"

namespace Game
{
    struct TestGLScene : public Chinstrap::Scene
    {
        Chinstrap::Renderer::Material material;
        TestGLScene();

        void OnBegin() override;
        void OnShutdown() override;
        void OnUpdate(float deltaTime) override;
        void OnRender(uint32_t currentFrame) override;

        void OnEvent(Chinstrap::Event &event) override;

        bool OnKeyPress(const Chinstrap::KeyPressedEvent &event) override;

        [[nodiscard]] std::string GetName() const override { return "TestGLScene"; }
    };
}
