#pragma once

#include "../memory/StringMap.h"
#include "../memory/CompHashMap.h"
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
// We define a resourceID by its virtual filepath.
// These aren't stored directly everywhere, but we use FilePath objects which hold
// the HashID's of the actual char[] in a Hash Map called filePathMap.
// We can then map every FilePath (meaning every HashID) to a Resource object,
// which holds all necessary runtime information.
//
// Resource objects hold pointers to the actual memory that holds the relevant data,
// like objects of struct Shader. For every supported resource type,
// there is a custom Memory pool data structure in place to hold these objects.
//
// We serialize (or save) all Resource objects held in our resourceData HashMap (by looking up their ID's).
// That means the contents of this HashMap are exactly all resources related to the application in question.
// In a non shipping build we can mark those as deleted through a bool variable, which will prevent them from being serialized.
//
// Tracking of these resources and their interferences when developing should be done through a developer-facing database,
// which works with an editor that is interfacing with this struct and a VCS, NOT by the engine- or game-code.
//
// In the shipping build, we want to serialize in binary to save on parsing time.
// Serializing in a custom text format during development makes debugging/changing (i.e. VCS tracking) a lot more straight forward
namespace Chinstrap::Resourcer
{
    struct ResourceManager;

    struct Resource
    {
        std::byte* pResource = nullptr; // Pointer to actual resource in memory
        // How many (for example scenes) are referencing this resource in order make good decisions on when to unload it
        uint32_t referenceCount = 0;

        ResourceType resourceType;
        ResourceID resourceID;
        static_assert(std::is_same_v<ResourceID, Memory::DevString::HashIDType>);

#ifndef CHIN_SHIPPING_BUILD
        // If we delete resources at runtime we still need this object around,
        // otherwise associated ResourceRef's will hold dangling pointers
        bool resourceDeleted = false;
#endif

        explicit Resource(ResourceID resourceID, ResourceType resourceType_arg)
            : resourceType(resourceType_arg), resourceID(resourceID) {}
    };

    // This function is automatically called when all ResourceRef's to a resource are deconstructed
    void UnloadResource(ResourceID resourceID, ResourceType resourceType, ResourceManager* callbackContext);

    // This function is automatically called by ResourceRef
    std::byte* GetCurrentResourcePtr(ResourceID resourceID, const ResourceManager* callbackContext);
}

struct Chinstrap::Resourcer::ResourceManager
{
#ifndef CHIN_SHIPPING_BUILD
    void CreateResource(const std::string_view& virtualFilePath, Memory::DevString& filePath_out);
    bool DeleteResource(const std::string_view& virtualFilePath, ResourceType resourceType);
    bool DeleteResource(const Memory::DevString& virtualFilePath, ResourceType resourceType);
#endif

    // TODO: I'm not really happy with loading resources on the fly in a non organized fashion
    //       What we're doing right now, is not scalable to multiple threads (jobs)
    // This will load the resource if it's not already
    bool GetResourceRef(const Memory::DevString& filePath, ResourceRef& resourceRef);

    bool Setup();
    void Cleanup();

    explicit ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;

private:
    void SerializeFilePaths();
    void SerializeFilePathsBinary();

    bool DeserializeFilePaths(std::vector<Memory::DevString>& materialPaths);
    void DeserializeFilePathsBinary();

    bool LoadResource(const Memory::DevString& filePath, Resource* resource, const ResourceRef& resourceRef);

public: /* Manage virtual file paths */

    Memory::StringMap filePathMap;

    // We want to serialize all resources contained at Save time by looking up their string values
    // by their hashID. We then write these virtual file paths in human-readable or binary.
    Memory::CompHashMap<Memory::DevString::HashIDType, Resource> resourceData;

public: /* Actually store resource data at runtime */
    Memory::MemoryPool<Renderer::Material> materialPool;
};