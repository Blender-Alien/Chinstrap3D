#include "ResourceManager.h"

#include "../ops/Logging.h"
#include "../rendering/RendererData.h"

using namespace Chinstrap::Resourcer;

ResourceRef& ResourceRef::operator=(const ResourceRef& other)
{
    if (this != &other)
    {
        this->resourceID = other.resourceID;
        if (referenceCount != nullptr)
        { // Very important, we created a new valid Ref!
            this->referenceCount = other.referenceCount;
            this->unloadPtr = other.unloadPtr;
            ++(*this->referenceCount);
        }
        return *this;
    }
    return *this;
}

ResourceRef::~ResourceRef()
{
    assert(*referenceCount > 0);

    --(*referenceCount);
    if (*referenceCount == 0)
    {
        unloadPtr->UnloadResource(resourceID);
    }
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

    resourceMetaData.emplace(filePath_out.GetHashID().value(), nextAvailableResourceID);
    ++nextAvailableResourceID;
}

#ifndef CHIN_SHIPPING_BUILD
bool ResourceManager::DeleteResource(const std::string_view& virtualFilePath)
{
    Memory::FilePath path;
    path.Create(virtualFilePath);
    return DeleteResource(path);
}
bool ResourceManager::DeleteResource(const Memory::FilePath& filePath)
{
    // Note that we're NOT actually onloading the resource itself.
    // Trying to access it will still fail now because there is no MetaData object associated with it anymore.
    // We don't really care about the wasted memory or invalid ResourceRef's, because we're not doing this
    // in shipping and the only required effect we need to achieve is that we're not serializing
    // this resource anymore, which should be the case when deleting it from resourceMetaData.
    if (resourceMetaData.erase(filePath.GetHashID().value()) != 1)
    {
        auto path = filePathMap.Lookup(filePath).value();
        CHIN_LOG_ERROR("Resource {} could not be deleted!", path);
        return false;
    }
    return true;
}
#endif

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
    resourceRef.unloadPtr = this;
    ++(*resourceRef.referenceCount);
    return GetResourceRefRet::SUCCESS;
}
#endif

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

void ResourceManager::UnloadResource(resourceIDType resourceId)
{
}

void ResourceManager::SaveAll()
{
}

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

    filePathMap.Cleanup();
    materialPool.Cleanup();
}

ResourceManager::ResourceManager()
    : nextAvailableResourceID(0)
{
}
