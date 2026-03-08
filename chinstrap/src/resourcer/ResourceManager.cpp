#include "ResourceManager.h"

#include "../ops/Logging.h"
#include "../rendering/RendererData.h"


Chinstrap::Resourcer::ResourceManager::ResourceManager()
    : aFilePaths(filepathAllocator)
{
}

#ifndef CHIN_SHIPPING_BUILD
Chinstrap::Resourcer::resourceIDType Chinstrap::Resourcer::ResourceManager::CreateResource(const std::string_view &name)
{
}
#endif

#ifndef CHIN_SHIPPING_BUILD
void Chinstrap::Resourcer::ResourceManager::DeleteResource(resourceIDType resourceID)
{
}
#endif

void Chinstrap::Resourcer::ResourceManager::Serialize()
{
}

void Chinstrap::Resourcer::ResourceManager::Deserialize()
{
}

bool Chinstrap::Resourcer::ResourceManager::Create()
{
    { // Load virtual file paths

    }

    { // Load Materials
        uint64_t numberOfSerializedResources = GetNumberOfSerializedResources<Renderer::Material>();
#ifdef CHIN_SHIPPING_BUILD
        numberOfSerializedResources;
#else // Start with headroom so we don't have to resize all the time
        numberOfSerializedResources *= 2;
#endif
        if (!materialPool.Setup(numberOfSerializedResources))
        {
            return false;
        }
    }

    return true;
}

void Chinstrap::Resourcer::ResourceManager::Cleanup()
{
    Serialize();

    materialPool.Cleanup();
}
