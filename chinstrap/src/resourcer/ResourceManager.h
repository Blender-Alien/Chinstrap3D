#pragma once

#include "../memory/MemoryPool.h"
#include "../memory/StackAllocator.h"
#include "../memory/StackArray.h"
#include "../memory/DevString.h"

namespace Chinstrap::Renderer { struct Material; }

/* Here, we want to handle loading and unloading all resourced needed by the game.
 * We want to track how many references are being held to any object, via counting
 * how many times we have given out uniqueID's to handle unloading/loading at the appropriate time.
 *
 * We also want to be able to serialize all uniqueID's and guarantee that they stay the same between program runs
 * and are automatically loadable. This means we need to be able to serialize and unserialize a list of all needed resources,
 * and provide an API to facilitate changes to this list.
 *
 * That is why our uniqueID's are hash values for 'virtual file paths' (see struct FilePath)
 * which are unique for every resource and thus define the resource. We can then store these ID's
 * in a dynamic array (struct StackArray) and when (de)serializing use the associated 'virtual file path'
 * to make the created file easily human understandable (making merge conflicts trivial).
 *
 * Tracking of these resources and their interferences when developing should be done through a developer-facing database,
 * which works with an editor that is interfacing with this struct and a VCS, NOT by the engine- or game-code.
 *
 * In the shipping build, we want to serialize in binary using already hashed ID's to save on parsing and hashing time
 */

namespace Chinstrap::Resourcer
{
    // Identify resources by given name and associated hashID
    typedef Memory::DevString resourceIDType;

    // Data only needed at runtime, will be thrown away when the program ends
    struct ResourceMetaData
    {
        void* pResource = nullptr; // Pointer to actual resource in memory
        uint32_t referenceCount = 0; // How many (for example scenes) are referencing this resource in order make good decisions on when to unload it
        bool loaded = false; // Is this resource loaded?
    };

    struct ResourceManager
    {
    public: /* Interface for developer and editor */
        resourceIDType CreateResource(const std::string_view& name);
        void DeleteResource(resourceIDType resourceID);

        bool CreateWithHeadroom(uint32_t additionalElementSpace);

    public: /* General interface for engine */
        void Serialize();

        bool Create();
        void Cleanup();

        explicit ResourceManager();
        ~ResourceManager() = default;
        ResourceManager(ResourceManager const&) = delete;
        ResourceManager& operator=(ResourceManager const&) = delete;

    private:
        void Deserialize();
        uint64_t GetNumberOfSerializedResources();

    private: /* Resource managing stuff */
        uint64_t currentResourceCapacity = 0;

        // We want to serialize the virtual path, not the FilePathID which should be used at runtime
        // UNLESS we are making a shipping build, which should serialize the FilePathID's in binary
        Memory::StackArray<Memory::FilePath> aFilePaths;
        Memory::StackAllocator filepathAllocator;

        // TODO: I'm not happy with std::unordered_map here, implement custom hash map that suits this use case

        // LUT for resourceID's to find actual MetaData
        std::unordered_map<Memory::FilePath::FilePathID, ResourceMetaData> resourceMetaData;

        // Hash map for every 'virtual file path' to avoid handling strings directly
        std::unordered_map<Memory::FilePath, Memory::FilePath::FilePathID> filepathHashMap;

    private: /* Actually store resource data at runtime */
        Memory::MemoryPool<Renderer::Material> materialPool;

    };
}