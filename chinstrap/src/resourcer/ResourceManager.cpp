#include "ResourceManager.h"

#include "../ops/Logging.h"
#include "../rendering/RendererData.h"

using namespace Chinstrap::Resourcer;

void Chinstrap::Resourcer::UnloadResource(const ResourceID resourceID, const ResourceType resourceType, ResourceManager* callbackContext)
{
    const auto it = callbackContext->resourceData.find(resourceID);
    const auto resource = &it->second;

    if (resource->pResource == nullptr)
    {
        // Resource wasn't loaded
        return;
    }
    switch (resourceType)
    {
    case ResourceType::SHADER:
        {
            auto ptr = reinterpret_cast<Renderer::Shader*>(resource->pResource);
            ptr->~Shader();
            callbackContext->shaderPool.Deallocate(&ptr);
            break;
        }
    case ResourceType::MATERIAL:
        {
            auto ptr = reinterpret_cast<Renderer::Material*>(resource->pResource);
            ptr->~Material();
            callbackContext->materialPool.Deallocate(&ptr);
            break;
        }
    default:
        {
            assert(false);
        }
    }
}

std::byte* Chinstrap::Resourcer::GetCurrentResourcePtr(const ResourceID resourceID, ResourceManager* callbackContext)
{
    const Resource* resource;
    const auto it = callbackContext->resourceData.find(resourceID);
    if (it != callbackContext->resourceData.end())
    {
        resource = &it->second;
    }
    else
    {
        return nullptr;
    }
    if (resource->pResource == nullptr)
    {
        return nullptr;
    }
    return resource->pResource;
}

#ifndef CHIN_SHIPPING_BUILD
void ResourceManager::CreateResource(const std::string_view& virtualFilePath, Memory::FilePath& filePath_out)
{
    auto result = filePathMap.Insert(filePath_out, virtualFilePath);
    if (result == Memory::FilePathMap::InsertRet::NO_KEY_CAPACITY
        || result == Memory::FilePathMap::InsertRet::NO_VALUE_CAPACITY)
    {
        // 20 is just some number I thought was okay, feel free to change it if you have a reason to
        filePathMap.GrowBy(20, std::nullopt);
        result = filePathMap.Insert(filePath_out, virtualFilePath);
    }
    if (result != Memory::FilePathMap::InsertRet::SUCCESS)
    {
        return;
    }

    resourceData.emplace(filePath_out.GetHashID().value(), filePath_out.GetHashID().value());
}
#endif

#ifndef CHIN_SHIPPING_BUILD
bool ResourceManager::DeleteResource(const std::string_view& virtualFilePath, ResourceType resourceType)
{
    Memory::FilePath path;
    path.Create(virtualFilePath);
    return DeleteResource(path, resourceType);
}
bool ResourceManager::DeleteResource(const Memory::FilePath& virtualFilePath, ResourceType resourceType)
{
    Resource* resource;
    const auto it = resourceData.find(virtualFilePath.GetHashID().value());
    if (it != resourceData.end())
    {
        resource = &it->second;
    }
    else
    {
        auto path = filePathMap.Lookup(virtualFilePath).value();
        CHIN_LOG_ERROR("Resource {} could not be deleted!", path);
        return false;
    }

    UnloadResource(resource->resourceID, resourceType, this);

    resource->resourceDeleted = true;
    return true;
}
#endif

bool ResourceManager::GetResourceRef(const Memory::FilePath& filePath, ResourceRef& resourceRef)
{
    if (!filePath.GetHashID().has_value())
    {
        return false;
    }

    Resource* resource;
    const auto it = resourceData.find(filePath.GetHashID().value());
    if (it != resourceData.end())
    {
        resource = &it->second;
    }
    else
    {
        return false;
    }

    if (resource->pResource == nullptr)
    { // Load resource
        auto virtualFilePath = filePathMap.Lookup(filePath);
        if (!virtualFilePath.has_value())
        {
            return false;
        }

        const char* OSPath = Memory::FilePath::ConvertToOSPath(virtualFilePath.value());

        switch (resourceRef.resourceType)
        {
        case ResourceType::SHADER:
            {
                const auto memory = shaderPool.Allocate();
                if (Renderer::ShaderLoader(memory, OSPath))
                    resource->pResource = reinterpret_cast<std::byte*>(memory);
                break;
            }
        case ResourceType::MATERIAL:
            {
                const auto memory = materialPool.Allocate();
                if (Renderer::MaterialLoader(memory, OSPath))
                    resource->pResource = reinterpret_cast<std::byte*>(memory);
                break;
            }
        default:
            {
                assert(false);
            }
        }
        delete[] OSPath;
    }
    resourceRef.resourceID = resource->resourceID;
    resourceRef.referenceCount = &resource->referenceCount;
    resourceRef.ptrResourceDeleted = &resource->resourceDeleted;
    resourceRef.callbackContext = this;
    resourceRef.unloadCallback = UnloadResource;
    resourceRef.getResourcePtr = GetCurrentResourcePtr;
    ++(*resourceRef.referenceCount);
    return true;
}

void ResourceManager::Serialize()
{
}
void ResourceManager::SerializeBinary()
{
}

void ResourceManager::Deserialize()
{
}
void ResourceManager::DeserializeBinary()
{
}

void ResourceManager::SaveAll()
{
}

bool ResourceManager::Setup()
{
    { // Load virtual file paths

        // TODO: Set size depending on number of deserialized file paths
        //       and maybe double that initially for a non shipping build
        filePathMap.Setup(10, std::nullopt);
        // Fill in deserialized filepaths
        filePathMap.EndSetup();
    }

    { // Load Materials
        uint64_t numberOfSerializedResources = 2; // TODO
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
    { // Load Shaders
        uint64_t numberOfSerializedResources = 2; // TODO
#ifdef CHIN_SHIPPING_BUILD
        numberOfSerializedResources;
#else // Start with headroom so we don't have to resize all the time
        numberOfSerializedResources *= 2;
#endif
        if (!shaderPool.Setup(numberOfSerializedResources))
        {
            return false;
        }
    }

    return true;
}

void ResourceManager::Cleanup()
{
    Serialize();

    filePathMap.Cleanup();
    materialPool.Cleanup();
    shaderPool.Cleanup();
}