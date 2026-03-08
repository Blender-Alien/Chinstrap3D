#include "FilePathMap.h"

#include "../ops/Logging.h"

using namespace Chinstrap::Memory;

FilePathMap::InsertRet FilePathMap::Insert(FilePath& filepath_arg, const std::string_view& inputString_arg)
{
    if (setupStatus == SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("Insert was used on a FilePathMap, but Setup was not begun!");
        assert(false);
        return InsertRet::BAD_REQUEST;
    }

    const FilePath::hashIDType argHashID = std::hash<std::string_view>()(inputString_arg);

    for (auto& keyIndex : keyArray)
    {
        // TODO: There must be a better way of finding the next free element of keyArray than to try them all with worst case O(n)
        //       Perhaps simply reserve() the current size and emplace_back(), then sort if applicable
        if (!keyIndex.has_value())
        {
            keyIndex.emplace(argHashID, valueStack);

            if (keyIndex.value().charArray.Allocate(inputString_arg.size() + 1) // + 1 for '\0'
                != true)
            {
                keyIndex.reset();
                return InsertRet::NO_VALUE_CAPACITY;
            }
            strcpy(keyIndex.value().charArray.data(), inputString_arg.data());
            keyIndex.value().charArray.data()[inputString_arg.size()] = '\0';

            // Give back hashID
            filepath_arg.hashID.emplace(argHashID);

            // TODO: Test speed difference between Setup then MergeSort approach and InsertionSort EveryTime even in Setup
            if (setupStatus == SetupStatus::SETUP_DONE)
            {
                InsertionSortKeyArray();
            } // else, we will sort later in EndSetup()

            return InsertRet::SUCCESS;
        }
        if (keyIndex.value().hashID == argHashID) { return InsertRet::COLLISION_OR_DUPLICATE; }
    }
    return InsertRet::NO_KEY_CAPACITY;
}

[[nodiscard]] std::optional<std::string_view> FilePathMap::Lookup(const FilePath& key_arg)
{
    if (setupStatus != SetupStatus::SETUP_DONE)
    {
        CHIN_LOG_WARN("A lookup in a FilePathMap was requested, before Setup was done!");
        assert(false);
        return std::nullopt;
    }
    { // Binary Search
        uint32_t left = 0;
        uint32_t right = keyArray.size() - 1;
        while (left <= right)
        {
            uint32_t middle = (left + right) / 2;

            if (keyArray.at(middle).has_value()
                && keyArray.at(middle).value().hashID == key_arg.hashID)
            {
                return std::string_view(keyArray.at(middle).value().charArray.data());
            }
            else if (keyArray.at(middle).has_value()
                && keyArray.at(middle).value().hashID < key_arg.hashID)
            {
                left = middle + 1;
                continue;
            }
            right = middle - 1;
        }
    }
    return std::nullopt;
}

#ifndef CHIN_SHIPPING_BUILD
void FilePathMap::Grow(uint32_t size_arg, std::optional<uint32_t> sizeInBytesHint_arg)
{
    if (keyArray.capacity() <= size_arg)
    {
        CHIN_LOG_WARN("A FilePathMap was instructed to grow, but the requested size was not larger.");
        assert(false);
        return;
    }
    keyArray.resize(size_arg);
    // TODO: Resize stackAllocator functionality
}
#endif

// Let's use std::sort for now, we'll properly implement this later
void FilePathMap::MergeSortKeyArray()
{
    std::vector<Key> tempKeys;
    tempKeys.reserve(keyArray.size());

    std::size_t numberOfActualElements = 0;
    for (auto & key : keyArray)
    {
        if (key.has_value())
        {
            tempKeys.push_back(key.value());
            ++numberOfActualElements;
        }
    }
    std::sort(tempKeys.begin(), tempKeys.end());

    for (std::size_t i = 0; i < numberOfActualElements; ++i)
    {
        keyArray[i].value() = tempKeys[i];
    }
}

void FilePathMap::InsertionSortKeyArray()
{
}

void FilePathMap::Setup(const uint32_t numberOfElements_arg, const std::optional<uint32_t> stringLengthHint_arg)
{
    if (setupStatus != SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("A FilePathMap was ordered to Setup(), but it already was or was in the process!");
        assert(false); // We have already setup!
        return;
    }
    setupStatus = SetupStatus::IN_SETUP;

    keyArray.resize(numberOfElements_arg);

    if (stringLengthHint_arg.has_value())
    {
        valueStack.Setup(numberOfElements_arg * sizeof(char[stringLengthHint_arg.value()]));
    }
    else
    {
        // Guess the average string size as 63 Characters to allocate ample space,
        // we choose 63 because it will end up being 64 with '\0' added at the end
        valueStack.Setup(numberOfElements_arg * sizeof(char[63]));
    }
}

void FilePathMap::Cleanup()
{
    valueStack.Cleanup();

    setupStatus = SetupStatus::NOT_BEGUN;
}

FilePathMap::FilePathMap()
    : keyArray(0)
{
}