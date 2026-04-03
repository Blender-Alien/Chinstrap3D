#pragma once

#include "chinstrap/src/Scene.h"
#include "chinstrap/src/rendering/RendererData.h"

namespace Game
{
    struct TestGLScene : public Chinstrap::Scene
    {
        Chinstrap::Resourcer::ResourceRef materialRef;

        // Temporary
        VkBuffer vertexBuffer;
        VmaAllocation vertexAllocation;
        VkBuffer indexBuffer;
        VmaAllocation indexAllocation;

        const std::vector<Chinstrap::Renderer::Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        explicit TestGLScene(Chinstrap::Resourcer::ResourceManager* pResourceManger_arg)
            : Scene(pResourceManger_arg), materialRef(Chinstrap::Resourcer::ResourceType::MATERIAL)
        {}

        void OnBegin() override;
        void OnShutdown() override;
        void OnUpdate(float deltaTime) override;
        void OnRender(uint32_t currentFrame) override;

        void OnEvent(Chinstrap::Event &event) override;
        //bool OnKeyPress(const Chinstrap::Event& event);

        [[nodiscard]] std::string GetName() const override { return "TestGLScene"; }
    };
}
