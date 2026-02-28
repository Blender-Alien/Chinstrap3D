#include "ResourceManager.h"

#include "../ops/Logging.h"
#include "../rendering/RendererData.h"


Chinstrap::Resourcer::ResourceManager::ResourceManager()
    : aFilePaths(filepathAllocator)
{
}

Chinstrap::Resourcer::resourceIDType Chinstrap::Resourcer::ResourceManager::CreateResource(const std::string_view &name)
{
}

void Chinstrap::Resourcer::ResourceManager::DeleteResource(resourceIDType resourceID)
{
}

void Chinstrap::Resourcer::ResourceManager::Serialize()
{
}

void Chinstrap::Resourcer::ResourceManager::Deserialize()
{
}

uint64_t Chinstrap::Resourcer::ResourceManager::GetNumberOfSerializedResources()
{
    return 0; // Not implemented yet, so no resource was serialized
}

bool Chinstrap::Resourcer::ResourceManager::Create()
{
    const uint64_t numberOfSerializedResources = GetNumberOfSerializedResources();
    currentResourceCapacity += numberOfSerializedResources;

    if (!materialPool.Setup(currentResourceCapacity))
    {
        return false;
    }

    return true;
}

bool Chinstrap::Resourcer::ResourceManager::CreateWithHeadroom(uint32_t additionalElementSpace)
{
    currentResourceCapacity = additionalElementSpace;
    return Create();
}

void Chinstrap::Resourcer::ResourceManager::Cleanup()
{
    Serialize();

    materialPool.Cleanup();
}
