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
    resource->pResource = nullptr;
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
    auto result = pFilePathMap->Insert(filePath_out, virtualFilePath);
    if (result == Memory::FilePathMap::InsertRet::NO_KEY_CAPACITY
        || result == Memory::FilePathMap::InsertRet::NO_VALUE_CAPACITY)
    {
        // 20 is just some number I thought was okay, feel free to change it if you have a reason to
        pFilePathMap->GrowBy(20, std::nullopt);
        result = pFilePathMap->Insert(filePath_out, virtualFilePath);
    }
    if (result != Memory::FilePathMap::InsertRet::SUCCESS)
    {
        return;
    }

    ResourceType resourceType;
    if (virtualFilePath.find(".material"))
    {
        resourceType = ResourceType::MATERIAL;
    }

    resourceData.emplace(std::piecewise_construct,
        std::forward_as_tuple(filePath_out.GetHashID().value()),
        std::forward_as_tuple(filePath_out.GetHashID().value(), resourceType));
}
#endif

#ifndef CHIN_SHIPPING_BUILD
bool ResourceManager::DeleteResource(const std::string_view& virtualFilePath, ResourceType resourceType)
{
    Memory::FilePath path;
    path.Hash(virtualFilePath);
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
        auto path = pFilePathMap->Lookup(virtualFilePath).value();
        CHIN_LOG_ERROR("Resource {} could not be deleted!", path);
        return false;
    }

    UnloadResource(resource->resourceID, resourceType, this);

    resource->resourceDeleted = true;
    return true;
}
#endif

bool ResourceManager::LoadResource(const Memory::FilePath& filePath, Resource* resource, ResourceRef& resourceRef)
{
    auto virtualFilePath = pFilePathMap->Lookup(filePath);
    if (!virtualFilePath.has_value())
    {
        return false;
    }

    std::unique_ptr<char> OSPath = Memory::FilePath::ConvertToOSPath(virtualFilePath.value());

    switch (resourceRef.resourceType)
    {
    case ResourceType::MATERIAL:
        {
            const auto memory = materialPool.Allocate();
            if (Renderer::MaterialLoader(memory, OSPath.get()))
                resource->pResource = reinterpret_cast<std::byte*>(memory);
            break;
        }
    default:
        {
            assert(false);
        }
    }
    return true;
}

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
    {
        if (!LoadResource(filePath, resource, resourceRef))
        {
            return false;
        }
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

bool ResourceManager::Setup(Memory::FilePathMap* filePathMap_arg, const std::string& appName)
{
    pFilePathMap = filePathMap_arg;

    std::vector<Memory::FilePath> materialPaths;

    { // Load virtual file paths
        std::string resourceListPath = appName + "/resources.txt";
        auto OSResourceListPath = Memory::FilePath::ConvertToOSPath(resourceListPath);

        std::ifstream fileStream(OSResourceListPath.get());
        if (!fileStream.is_open())
        {
            return false;
        }

        std::vector<std::string> paths;
        std::string line;
        while (std::getline(fileStream, line))
        {
            paths.push_back(std::move(line));
        }

        {
            std::ifstream file(OSResourceListPath.get(), std::ios::binary | std::ios::ate);
            std::size_t sizeInBytes = file.tellg();
#ifdef CHIN_SHIPPING_BUILD
            pFilePathMap->Setup(paths.size(), sizeInBytes);
#else
            pFilePathMap->Setup(paths.size() * 2, sizeInBytes * 2);
#endif
        }

        for (auto& path : paths)
        {
            Memory::FilePath filePath;
            auto ret = pFilePathMap->Insert(filePath, path);
            assert(ret == Memory::FilePathMap::InsertRet::SUCCESS);

            if (path.find(".material"))
            {
                materialPaths.push_back(filePath);
            }
            // etc ...
        }

        pFilePathMap->EndSetup();
    }

    { // Load Materials
        uint64_t numberOfSerializedResources = std::max(materialPaths.size(), static_cast<std::size_t>(3));
#ifndef CHIN_SHIPPING_BUILD // Start with headroom so we don't have to resize all the time
        numberOfSerializedResources *= 2;
#endif
        if (!materialPool.Setup(numberOfSerializedResources))
        {
            return false;
        }
        for (auto& materialPath : materialPaths)
        {
            resourceData.emplace(std::piecewise_construct,
                std::forward_as_tuple(materialPath.GetHashID().value()),
                std::forward_as_tuple(materialPath.GetHashID().value(), ResourceType::MATERIAL));
        }
    }

    return true;
}

void ResourceManager::Cleanup()
{
    Serialize();

    for (auto& resource : resourceData)
    {
        UnloadResource(resource.second.resourceID, resource.second.resourceType, this);
    }
    materialPool.Cleanup();
}