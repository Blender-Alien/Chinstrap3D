#pragma once

// TODO: Get rid of MaterialManager, use ResourceManager instead

#include "../rendering/RendererData.h"

namespace Chinstrap::Resourcer
{
    struct MaterialManager
    {
        // We shouldn't give out references, handle this with ID's when seriously writing this
        Renderer::Material* GetMaterial();

        void MakeMaterial();

        explicit MaterialManager();
        void Destroy();

        MaterialManager(MaterialManager const&) = delete;
        MaterialManager& operator=(MaterialManager const&) = delete;
    private:

        Renderer::Material* material = nullptr;
    };
}