#pragma once

#include "StackAllocator.h"
#include "StackArray.h"

#include "DevString.h"

namespace Chinstrap::Memory
{
    std::unique_ptr<char> ConvertToOSPath(const std::string_view& virtualFilePath);
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
struct Chinstrap::Memory::StringMap
{
    enum class InsertRet
    {
        SUCCESS, COLLISION_OR_DUPLICATE,
        NO_KEY_CAPACITY, NO_VALUE_CAPACITY, BAD_REQUEST
    };

    // Insert a new FilePath into Map given a string as it's actual value
    [[nodiscard]] InsertRet Insert(DevString& string_arg, const std::string_view& inputString_arg);

    /**
     * TODO: This is not thread safe!
     * @return
     * Get CURRENT address of associated string, immediately use after acquiring and then discard
     * NEVER save this for later use, it can be invalidated!!!
     */
    [[nodiscard]] std::optional<std::string_view> Lookup(const DevString& key_arg) const;

    /**
     * @param byNumberOfElements_arg How many more elements should be able to fit?
     * @param avgStringLengthHint_arg How much more space should be allocated per new element?
     * If left empty, the average of previous value sizes will be used.
     */
    void GrowBy(uint32_t byNumberOfElements_arg, std::optional<uint32_t> avgStringLengthHint_arg);

    [[nodiscard]] std::optional<std::string_view> Iterate(uint32_t index) const;

    void Setup(uint32_t numberOfElements, uint32_t totalValuesSize);
    void EndSetup()
    {
        MergeSortKeyArray();
        setupStatus = SetupStatus::SETUP_DONE;
    }

    void Cleanup();

    explicit StringMap();

private:
    enum class SetupStatus { NOT_BEGUN, IN_SETUP, SETUP_DONE };

    SetupStatus setupStatus = SetupStatus::NOT_BEGUN;

    // Hold one hashID and associated string data
    struct Key
    {
        DevString::HashIDType hashID;
        Memory::StackArray<char> charArray;

        bool operator<(const Key& key_arg) const
        {
            return hashID < key_arg.hashID;
        }

        explicit Key(DevString::HashIDType hashID_arg, Memory::StackAllocator* allocator_arg)
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
};
