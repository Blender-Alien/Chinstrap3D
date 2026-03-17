#include "ResourceManager.h"

#include "../rendering/RendererData.h"


using namespace Chinstrap::Resourcer;

ResourceManager::ResourceManager()
    : aFilePaths(filepathAllocator)
{
}

#ifndef CHIN_SHIPPING_BUILD
resourceIDType ResourceManager::CreateResource(const Memory::FilePath& filePath)
{
}
#endif

#ifndef CHIN_SHIPPING_BUILD
void ResourceManager::DeleteResource(resourceIDType resourceID)
{
}

ResourceManager::GetResourceRefRet ResourceManager::GetResourceRef(const Memory::FilePath& filePath, ResourceRef& resourceRef)
{
    if (!filePath.GetHashID().has_value())
    {
        return GetResourceRefRet::FILE_PATH_INVALID;
    }

    const ResourceMetaData* resource = nullptr;
    try
    {
        resource = &resourceMetaData.at(filePath.GetHashID().value());
    } catch (std::out_of_range& e)
    {
        return GetResourceRefRet::UNKNOWN_RESOURCE;
    }

    if (resource->pResource == nullptr)
    { // Load resource
        auto virtualFilePath = filePathMap.Lookup(filePath);
        if (!virtualFilePath.has_value())
        {
            return GetResourceRefRet::FILE_PATH_INVALID;
        }
        char OSPath[virtualFilePath.value().size()];
        Memory::FilePath::ConvertToOSPath(virtualFilePath.value(), OSPath);

        // TODO: LOAD SHIT
        return GetResourceRefRet::SUCCESS;
    }

    resourceRef.resourceID = resource->resourceID;
    // Const cast here because std::unordered_map is annoying
    resourceRef.referenceCount = const_cast<uint32_t*>(&resource->referenceCount);
    ++(*resourceRef.referenceCount);
    return GetResourceRefRet::SUCCESS;
}

ResourceManager::GetResourceCurrentPtrRet ResourceManager::GetResourceCurrentPtr(const resourceIDType resourceID, void*& pointer) const
{
    const ResourceMetaData* resource;
    try
    {
        resource = &resourceMetaData.at(resourceID);
    } catch (std::out_of_range& e)
    {
        return GetResourceCurrentPtrRet::UNKNOWN_RESOURCE;
    }
    if (resource->pResource == nullptr)
    {
        return GetResourceCurrentPtrRet::RESOURCE_UNLOADED;
    }
    pointer = resource->pResource;
    return GetResourceCurrentPtrRet::SUCCESS;
}
#endif

void ResourceManager::Serialize()
{
}
#ifdef CHINSHIPPING_BUILD
void Chinstrap::Resourcer::ResourceManager::SerializeBinary()
{
}
#endif

void ResourceManager::Deserialize()
{
}
#ifdef CHINSHIPPING_BUILD
void Chinstrap::Resourcer::ResourceManager::DeserializeBinary()
{
}
#endif

bool ResourceManager::Setup()
{
    { // Load virtual file paths

        // TODO: Set size depending on number of deserialized filepaths
        //       and maybe double that initially for a non shipping build
        filePathMap.Setup(10, std::nullopt);
        // Fill in deserialized filepaths
        filePathMap.EndSetup();
    }

    { // Load Materials
        uint64_t numberOfSerializedResources = 0; // TODO
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

void ResourceManager::Cleanup()
{
    Serialize();

    filepathAllocator.Cleanup();
    filePathMap.Cleanup();
    materialPool.Cleanup();
}
