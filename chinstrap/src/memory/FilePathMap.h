#pragma once

#include "StackAllocator.h"
#include "StackArray.h"

namespace Chinstrap::Memory
{
    struct FilePathMap;

    // We handle filepath relative to the project working directory like this
    // '/app/resources/sounds/beepboop.wav' called a 'VirtualPath'
    // This can be requested as an OS specific absolute path 'OSPath',
    // in order to load or stream the file in question
    struct FilePath
    {
        typedef std::size_t HashID; // std::hash returns a value of type std::size_t

        [[nodiscard]] std::optional<HashID> GetHashID() const
        {
            return hashID;
        }

        /**
         * @return char* to char[] that needs to be deleted by caller,
         * holds absolute OS-specific path.
         */
        static std::unique_ptr<char> ConvertToOSPath(const std::string_view& virtualFilePath);

        void Create(const std::string_view& virtualFilePath)
        {
            assert(!hashID.has_value());
            hashID.emplace(std::hash<std::string_view>()(virtualFilePath));
        }
    private:
        friend FilePathMap;
        std::optional<HashID> hashID;
    };
}

// Store multiple Key's with hashID and Memory::StackArray<char> each in a
// (growable when not in shipping build) heap allocated array,
// where Memory::StackArray<char>'s use valueStack to allocate their actual 'string' data,
// a (growable when not in shipping build) heap allocated Stack 'Memory::StackAllocator'.
//
// This allows us to sort the keyArray by hashID and allows fast lookup at the price of sorting/reallocation
// costs of the keyArray when entering strings randomly at runtime (which we do NOT allow in a shipping build).
//
// In the future when we have a Job system (concurrency),
// we might need to rethink how we insert into the map and sort the keyArray.
struct Chinstrap::Memory::FilePathMap
{
    enum class InsertRet
    {
        SUCCESS, COLLISION_OR_DUPLICATE,
        NO_KEY_CAPACITY, NO_VALUE_CAPACITY, BAD_REQUEST
    };

    // Insert a new FilePath into Map given a string as it's actual value
    [[nodiscard]] InsertRet Insert(FilePath& filepath_arg, const std::string_view& inputString_arg);

    // Get CURRENT address of associated string, immediately use after acquiring and then discard
    // NEVER save this pointer for later use, it can be invalidated!!!
    [[nodiscard]] std::optional<std::string_view> Lookup(const FilePath& key_arg) const;

#ifndef CHIN_SHIPPING_BUILD
    bool GrowBy(uint32_t byNumberOfElements_arg, std::optional<uint32_t> stringLengthHint_arg);
#endif

    [[nodiscard]] std::optional<std::string_view> Iterate(uint32_t index) const;

    void Setup();
    // Call this after inserting multiple deserialized filepath's
    void EndSetup()
    {
        MergeSortKeyArray();
        setupStatus = SetupStatus::SETUP_DONE;
    }

    void Cleanup();

    explicit FilePathMap();

private:
    enum class SetupStatus { NOT_BEGUN, IN_SETUP, SETUP_DONE };

    SetupStatus setupStatus = SetupStatus::NOT_BEGUN;

    // Hold one hashID and associated string data
    struct Key
    {
        FilePath::HashID hashID;
        Memory::StackArray<char> charArray;

        bool operator<(const Key& key_arg) const
        {
            return hashID < key_arg.hashID;
        }

        explicit Key(FilePath::HashID hashID_arg, Memory::StackAllocator& allocator_arg)
            : hashID(hashID_arg), charArray(allocator_arg) {}
    };

    void InsertionSortKeyArray();

    void MergeSortKeyArray();
    static void MergeSort(std::vector<std::optional<Key>>& array, std::size_t p, std::size_t r);
    static void Merge(std::vector<std::optional<Key>>& array, std::size_t leftIndex, std::size_t middleIndex,
                      std::size_t rightIndex);

    // Store our char arrays in this stack
    Memory::StackAllocator valueStack;

    std::size_t keyArrayHasValueSize;
    std::vector<std::optional<Key>> keyArray;

    friend FilePath;
    inline static std::string* programPath;
    inline static std::size_t rootIndex;
};
