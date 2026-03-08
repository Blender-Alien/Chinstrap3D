#pragma once

#include "StackAllocator.h"
#include "StackArray.h"

#include <cstring>
#include <bits/std_thread.h>

namespace Chinstrap::Memory
{
    // We handle filepath relative to the project working directory like this
    // '/resources/sounds/beepboop.wav' called a 'VirtualPath'
    // This can be requested as an OS specific path 'OSPath',
    // in order to load or stream the file in question
    struct FilePath
    {
        typedef std::size_t hashIDType; // std::hash returns a value of type std::size_t

        std::optional<hashIDType> hashID;

        // Convert to current OS specific path
        [[nodiscard]] const std::string_view ConvertToOSPath() const;
    };

    struct FilePathMap;
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
    [[nodiscard]] std::optional<std::string_view> Lookup(const FilePath& key_arg);

#ifndef CHIN_SHIPPING_BUILD
    void Grow(uint32_t size_arg, std::optional<uint32_t> sizeInBytesHint_arg);
#endif

    void Setup(uint32_t numberOfElements_arg, std::optional<uint32_t> stringLengthHint_arg);
    // Call this after inserting multiple deserialized filepath's
    void EndSetup()
    {
        MergeSortKeyArray();
        setupStatus = SetupStatus::SETUP_DONE;
    }

    void Cleanup();

    explicit FilePathMap();

private:
    void MergeSortKeyArray();
    void InsertionSortKeyArray();

    enum class SetupStatus { NOT_BEGUN, IN_SETUP, SETUP_DONE };
    SetupStatus setupStatus = SetupStatus::NOT_BEGUN;

    // Hold one hashID and associated string data
    struct Key
    {
        FilePath::hashIDType hashID;
        Memory::StackArray<char> charArray;

        bool operator<(const Key& key_arg) const
        {
            return hashID < key_arg.hashID;
        }

        explicit Key(FilePath::hashIDType hashID_arg, Memory::StackAllocator& allocator_arg)
            : hashID(hashID_arg), charArray(allocator_arg) {}
    };

    // Store our char arrays in this stack
    Memory::StackAllocator valueStack;

    // This can also be a stack array really, but for now in a non shipping build, it's more convenient to resize
    std::vector<std::optional<Key>> keyArray;
};