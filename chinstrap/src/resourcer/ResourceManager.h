#pragma once

#include "../memory/MemoryPool.h"
#include "../memory/StackAllocator.h"
#include "../memory/StackArray.h"
#include "../memory/FilePathMap.h"

namespace Chinstrap::Renderer { struct Material; }

// Here, we want to handle loading and unloading all resourced needed by the game.
// We want to track how many references are being held to any object, via counting
// how many times we have given out uniqueID's to handle unloading/loading at the appropriate time.
//
// We also want to be able to serialize all uniqueID's and guarantee that they stay the same between program runs
// and are automatically loadable. This means we need to be able to serialize and unserialize a list of all needed resources,
// and provide an API to facilitate changes to this list.
//
// That is why our uniqueID's are hash values for 'virtual file paths' (see struct FilePath)
// which are unique for every resource and thus define the resource. We can then store these ID's
// in a dynamic array (struct StackArray) and when (de)serializing use the associated 'virtual file path'
// to make the created file easily human understandable (making merge conflicts trivial).
//
// Tracking of these resources and their interferences when developing should be done through a developer-facing database,
// which works with an editor that is interfacing with this struct and a VCS, NOT by the engine- or game-code.
//
// In the shipping build, we want to serialize in binary using already hashed ID's to save on parsing and hashing time
namespace Chinstrap::Resourcer
{
    // Identify resources by given name and associated hashID
    typedef std::size_t resourceIDType;

    // Data only needed at runtime, will be thrown away when the program ends
    struct ResourceMetaData
    {
        void* pResource = nullptr; // Pointer to actual resource in memory
        uint32_t referenceCount = 0;
        // How many (for example scenes) are referencing this resource in order make good decisions on when to unload it
        bool loaded = false; // Is this resource loaded?
    };

    struct ResourceManager;
}

struct Chinstrap::Resourcer::ResourceManager
{
#ifndef CHIN_SHIPPING_BUILD
    resourceIDType CreateResource(const std::string_view& name);
    void DeleteResource(resourceIDType resourceID);
#endif

    void Serialize();

    bool Create();
    void Cleanup();

    explicit ResourceManager();
    ~ResourceManager() = default;
    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;

private:
    void Deserialize();

    template <typename resourceType>
    uint64_t GetNumberOfSerializedResources()
    {
        return 0; // To be implemented
    }

#ifndef CHIN_SHIPPING_BUILD
    template <typename resourceType>
    void Grow();
#endif

private: /* Manage virtual file paths */

    Memory::StackArray<Memory::FilePath> aFilePaths; // Serialized (as virtual paths in binary)
    Memory::StackAllocator filepathAllocator;

    // Hash map for every 'virtual file path' to avoid handling strings directly
    Memory::FilePathMap filePathMap;

    // TODO: This should also be a custom HashMap
    // LUT for resourceID's to find actual MetaData
    std::unordered_map<Memory::FilePath::hashIDType, ResourceMetaData> resourceMetaData;

private: /* Actually store resource data at runtime */
    Memory::MemoryPool<Renderer::Material> materialPool;
};
