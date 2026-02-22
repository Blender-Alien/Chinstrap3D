#include "MaterialManager.h"

#include "../Application.h"
#include "../rendering/VulkanFunctions.h"

Chinstrap::Resourcer::MaterialManager::MaterialManager()
{
}


void Chinstrap::Resourcer::MaterialManager::MakeMaterial()
{
    if (material != nullptr)
    {
        return;
    }
    material = new Renderer::Material(
            Application::App::GetVulkanContext(),
            readFile("../../../chinstrap/res/shaders/BasicVertex.spv"),
            readFile("../../../chinstrap/res/shaders/BasicFragment.spv")
    );
}

void Chinstrap::Resourcer::MaterialManager::Destroy()
{
    material->Cleanup();
    delete material;
}

Chinstrap::Renderer::Material*  Chinstrap::Resourcer::MaterialManager::GetMaterial()
{
    return material;
}
