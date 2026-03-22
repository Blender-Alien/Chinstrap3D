#pragma once

#include "../memory/FilePathMap.h"
#include "../memory/MemoryPool.h"

#include "ResourceRef.h"

namespace Chinstrap::Renderer { struct Material; }
namespace Chinstrap::Renderer { struct Shader; }

// Here, we want to handle loading and unloading all resourced needed by the game.
// We track references by using ResourceRef object which we don't give out, but fill
// in to hold valid data. When the these objects get destroyed resourceCount will automatically be decremented.
//
// We also want to be able to serialize resources with some kind of ID and guarantee that they stay the same between program runs.
// This means we need to be able to serialize and unserialize a list (not necessarily stored as a list or array) of all resources,
// and provide an API to facilitate changes to this list.
//
// We have resourceIDType which are given out at runtime that simply serve to facilitate the ResourceRef system.
// To load, delete, create etc. a virtual file path is required.
// These aren't stored directly everywhere, but we use FilePath objects which hold a
// HashID of the virtual file path that has a stored string value in a HashMap filePathMap
// when the resource was created or deserialized.
//
// We serialize (or save) all ResourceMetaData objects held in our resourceMetaData HashMap (by looking up their ID's).
// That means the contents of this HashMap are exactly all resources related to the application in question.
//
// Tracking of these resources and their interferences when developing should be done through a developer-facing database,
// which works with an editor that is interfacing with this struct and a VCS, NOT by the engine- or game-code.
//
// In the shipping build, we want to serialize in binary to save on parsing time.
// Serializing in a custom text format during development makes debugging/changing (i.e. VCS tracking) a lot more straight forward
namespace Chinstrap::Resourcer
{
    struct ResourceManager;

    // Data only needed at runtime, will be thrown away when the program ends
    struct ResourceMetaData
    {
        std::byte* pResource = nullptr; // Pointer to actual resource in memory
        // How many (for example scenes) are referencing this resource in order make good decisions on when to unload it
        uint32_t referenceCount = 0;

        resourceIDType resourceID;
        static_assert(std::is_same_v<resourceIDType, Memory::FilePath::hashIDType>);

#ifndef CHIN_SHIPPING_BUILD
        // If we delete resources at runtime we still need this object around,
        // otherwise associated ResourceRef's will hold dangling pointers
        bool resourceDeleted = false;
#endif

        explicit ResourceMetaData(resourceIDType resourceID)
            : resourceID(resourceID) {}
    };

    // This function is automatically called when all ResourceRef's to a resource are deconstructed
    void UnloadResource(resourceIDType resourceID, ResourceType resourceType, ResourceManager* callbackContext);
}

struct Chinstrap::Resourcer::ResourceManager
{
#ifndef CHIN_SHIPPING_BUILD
    void CreateResource(const std::string_view& virtualFilePath, Memory::FilePath& filePath_out);
    bool DeleteResource(const std::string_view& virtualFilePath, ResourceType resourceType);
    bool DeleteResource(const Memory::FilePath& filePath, ResourceType resourceType);
#endif

    // TODO: I'm not really happy with loading resources on the fly in a non organized fashion
    //       What we're doing right now, is not scalable to multiple threads (jobs)
    // This will load the resource if it's not already
    bool GetResourceRef(const Memory::FilePath& filePath, ResourceRef& resourceRef,
                                     std::byte* (*ResourceLoader)(std::byte* dataPtr, std::string_view OSFilePath));

    template<typename resourceType>
    [[nodiscard]] resourceType* GetResCurrentPtr(resourceIDType resourceID) const
    {
        const ResourceMetaData* resource;
        const auto it = resourceMetaData.find(resourceID);
        if (it != resourceMetaData.end())
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
        return reinterpret_cast<resourceType*>(resource->pResource);
    }

    void SaveAll();

    bool Setup();
    void Cleanup();

    explicit ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;

private:
    void Serialize();
    void SerializeBinary();

    void Deserialize();
    void DeserializeBinary();

public: /* Manage virtual file paths */

    // Hash map for every 'virtual file path' to avoid handling strings directly
    Memory::FilePathMap filePathMap;

    // TODO: This should also be a custom HashMap
    // We want to serialize all resources contained at Save time by looking up their string values
    // by their hashID. We then write these virtual file paths in human readable or binary.
    std::unordered_map<Memory::FilePath::hashIDType, ResourceMetaData> resourceMetaData;

public: /* Actually store resource data at runtime */
    Memory::MemoryPool<Renderer::Material> materialPool;
    Memory::MemoryPool<Renderer::Shader> shaderPool;
};
