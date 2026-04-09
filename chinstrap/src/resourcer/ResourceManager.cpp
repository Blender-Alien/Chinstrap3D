#include "ResourceManager.h"

#include "../Application.h"
#include "../ops/Logging.h"
#include "../rendering/RendererData.h"

using namespace Chinstrap::Resourcer;

void Chinstrap::Resourcer::UnloadResource(const ResourceID resourceID, const ResourceType resourceType, ResourceManager* callbackContext)
{
    auto opt = callbackContext->resourceData.Lookup(resourceID);
    ENSURE_OR_RETURN((opt.has_value() && opt.value()->pResource != nullptr));
    Resource* resource = opt.value();

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
    const auto opt = callbackContext->resourceData.Lookup(resourceID);
    if (!opt.has_value())
    {
        return nullptr;
    }
    Resource* resource = opt.value();

    if (resource->pResource == nullptr)
    {
        return nullptr;
    }
    return resource->pResource;
}

#ifndef CHIN_SHIPPING_BUILD
void ResourceManager::CreateResource(const std::string_view& virtualFilePath, Memory::DevString& filePath_out)
{
    auto result = filePathMap.Insert(filePath_out, virtualFilePath);
    if (result == Memory::StringMap::InsertRet::NO_KEY_CAPACITY
        || result == Memory::StringMap::InsertRet::NO_VALUE_CAPACITY)
    {
        // 20 is just some number I thought was okay, feel free to change it if you have a reason to
        filePathMap.GrowBy(20, std::nullopt);
        CHIN_LOG_WARN("ResourceManager: filePathMap needed to grow!");
        result = filePathMap.Insert(filePath_out, virtualFilePath);
    }
    if (result != Memory::StringMap::InsertRet::SUCCESS)
    {
        return;
    }

    ResourceType resourceType;
    if (virtualFilePath.find(".material"))
    {
        resourceType = ResourceType::MATERIAL;
    }
    else
    {
        ENSURE_OR_RETURN_MSG((false), "Failed to create resource! File extension not recognized: {}", virtualFilePath);
    }

    const auto resource = Resource(filePath_out.GetHashID().value(), resourceType);
    auto ret = resourceData.Insert(filePath_out.GetHashID().value(), resource);
    if (ret == Memory::HashInsertRet::NO_CAPACITY)
    {
        resourceData.GrowBy(resourceData.keyArrayHasValueSize * 2);
        CHIN_LOG_WARN("ResourceManager: resourceData needed to grow!");
        ret = resourceData.Insert(filePath_out.GetHashID().value(), resource);
    }
    ENSURE_OR_RETURN((ret == Memory::HashInsertRet::SUCCESS));
}
#endif

#ifndef CHIN_SHIPPING_BUILD
bool ResourceManager::DeleteResource(const std::string_view& virtualFilePath, ResourceType resourceType)
{
    Memory::DevString path;
    path.Hash(virtualFilePath);
    return DeleteResource(path, resourceType);
}
bool ResourceManager::DeleteResource(const Memory::DevString& virtualFilePath, ResourceType resourceType)
{
    const auto opt = resourceData.Lookup(virtualFilePath.GetHashID().value());
    if (!opt.has_value())
    {
        auto path = filePathMap.Lookup(virtualFilePath).value();
        CHIN_LOG_ERROR("Resource {} could not be deleted!", path);
        return false;
    }
    Resource* resource = opt.value();

    UnloadResource(resource->resourceID, resourceType, this);

    CHIN_LOG_INFO("Resource {} was deleted", filePathMap.Lookup(virtualFilePath).value());
    resource->resourceDeleted = true;
    return true;
}
#endif

bool ResourceManager::LoadResource(const Memory::DevString& filePath, Resource* resource, ResourceRef& resourceRef)
{
    auto virtualFilePath = filePathMap.Lookup(filePath);
    ENSURE_OR_RETURN_FALSE(virtualFilePath.has_value());

    std::unique_ptr<char> OSPath = Memory::ConvertToOSPath(virtualFilePath.value());

    switch (resourceRef.resourceType)
    {
    case ResourceType::MATERIAL:
        {
            const auto memory = materialPool.Allocate();
            ENSURE_OR_RETURN_FALSE_MSG(Renderer::MaterialLoader(memory, OSPath.get()), "Failed to load material: {}", OSPath.get());
            CHIN_LOG_INFO("Successfully loaded material: {}", OSPath.get());
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

bool ResourceManager::GetResourceRef(const Memory::DevString& filePath, ResourceRef& resourceRef)
{
    ENSURE_OR_RETURN_FALSE(filePath.GetHashID().has_value());

    auto opt = resourceData.Lookup(filePath.GetHashID().value());
    ENSURE_OR_RETURN_FALSE((opt.has_value()));
    Resource* resource = opt.value();

    if (resource->pResource == nullptr)
    {
        ENSURE_OR_RETURN_FALSE(LoadResource(filePath, resource, resourceRef));
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

void ResourceManager::SerializeFilePaths()
{
    // TODO: This can wait until we have an editor to create stuff with
}
void ResourceManager::SerializeFilePathsBinary()
{
}

bool ResourceManager::DeserializeFilePaths(std::vector<Memory::DevString>& materialPaths)
{
    std::string resourceListPath = Application::App::config.appName + "/resources.chin";
    auto OSResourceListPath = Memory::ConvertToOSPath(resourceListPath);

    std::ifstream fileStream(OSResourceListPath.get());
    ENSURE_OR_RETURN_FALSE(fileStream.is_open());

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
        filePathMap.Setup(paths.size(), sizeInBytes);
        resourceData.Setup(paths.size());
#else
        filePathMap.Setup(paths.size() * 2, sizeInBytes * 2);
        resourceData.Setup(paths.size() * 2);
#endif
    }

    for (auto& path : paths)
    {
        Memory::DevString filePath;
        auto ret = filePathMap.Insert(filePath, path);
        ENSURE_OR_RETURN_FALSE((ret == Memory::StringMap::InsertRet::SUCCESS));

        if (path.find(".material"))
        {
            materialPaths.push_back(filePath);
        }
        // etc ...
    }

    filePathMap.EndSetup();
    return true;
}

void ResourceManager::DeserializeFilePathsBinary()
{
}

bool ResourceManager::Setup()
{
    std::vector<Memory::DevString> materialPaths;
    ENSURE_OR_RETURN_FALSE(DeserializeFilePaths(materialPaths));

    { // Load Materials
        uint64_t numberOfSerializedResources = std::max(materialPaths.size(), static_cast<std::size_t>(3));
#ifndef CHIN_SHIPPING_BUILD // Start with headroom so we don't have to resize all the time
        numberOfSerializedResources *= 2;
#endif
        ENSURE_OR_RETURN_FALSE(materialPool.Setup(numberOfSerializedResources));
        for (auto& materialPath : materialPaths)
        {
            auto resource = Resource(materialPath.GetHashID().value(), ResourceType::MATERIAL);
            auto ret = resourceData.Insert(materialPath.GetHashID().value(), resource);
            if (ret == Memory::HashInsertRet::NO_CAPACITY)
            {
                resourceData.GrowBy(resourceData.keyArrayHasValueSize * 2);
                CHIN_LOG_WARN("ResourceManager: resourceData needed to grow!");
                ret = resourceData.Insert(materialPath.GetHashID().value(), resource);
            }
            ENSURE_OR_RETURN_FALSE((ret == Memory::HashInsertRet::SUCCESS));
        }
    }

    resourceData.EndSetup();

    return true;
}

void ResourceManager::Cleanup()
{
    SerializeFilePaths();

    for (uint32_t i = 0; i < resourceData.keyArrayHasValueSize; ++i)
    {
        const auto resource = resourceData.Iterate(i).value();
        UnloadResource(resource->resourceID, resource->resourceType, this);
    }
    materialPool.Cleanup();
    filePathMap.Cleanup();
    resourceData.Cleanup();
}