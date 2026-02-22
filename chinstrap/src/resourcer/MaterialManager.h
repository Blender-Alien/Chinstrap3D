#pragma once

/* I'm not sure how to properly handle this yet,
 * but for now I need a stopgap so that scenes can use materials
 * and don't have to clean up the associated vulkan resources themselves
 */

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