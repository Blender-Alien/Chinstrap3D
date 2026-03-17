#include "FilePathMap.h"

#include "../ops/Logging.h"

using namespace Chinstrap::Memory;

FilePathMap::InsertRet FilePathMap::Insert(FilePath& filepath_arg, const std::string_view& inputString_arg)
{
    if (setupStatus == SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("Insert was used on a FilePathMap, but Setup was not begun!");
        return InsertRet::BAD_REQUEST;
    }

    const FilePath::hashIDType argHashID = std::hash<std::string_view>()(inputString_arg);

    for (auto& keyIndex : keyArray)
    {
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

            if (setupStatus == SetupStatus::SETUP_DONE)
            {
                InsertionSortKeyArray();
            } // else, we will sort later in EndSetup()

            ++keyArrayHasValueSize;
            return InsertRet::SUCCESS;
        }
        if (keyIndex.value().hashID == argHashID) { return InsertRet::COLLISION_OR_DUPLICATE; }
    }
    return InsertRet::NO_KEY_CAPACITY;
}

[[nodiscard]] std::optional<std::string_view> FilePathMap::Lookup(const FilePath& key_arg) const
{
    if (setupStatus != SetupStatus::SETUP_DONE)
    {
        CHIN_LOG_WARN("A lookup in a FilePathMap was requested, before Setup was done!");
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
                return std::string_view(keyArray.at(middle).value().charArray.dataConst());
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
bool FilePathMap::GrowTo(uint32_t numberOfElements_arg, std::optional<uint32_t> stringLengthHint_arg)
{
    if (keyArray.capacity() >= numberOfElements_arg)
    {
        CHIN_LOG_WARN("A FilePathMap was instructed to grow, but the requested size was not larger.");
        return false;
    }
    keyArray.resize(numberOfElements_arg);

    StackAllocator newValueStack;
    if (stringLengthHint_arg.has_value())
    {
        newValueStack.Setup(numberOfElements_arg * sizeof(char[stringLengthHint_arg.value()]));
    }
    else
    {
        // Guess the average string size as 64 Characters to allocate ample space
        newValueStack.Setup(numberOfElements_arg * sizeof(char[64]));
    }

    for (auto& keyIndex : keyArray)
    {
        if (!keyIndex.has_value())
            continue;

        Memory::StackArray<char> newArray(newValueStack);
        if (!newArray.Allocate(keyIndex.value().charArray.capacity()))
            return false;

        strcpy(newArray.data(), keyIndex.value().charArray.data());

        keyIndex.value().charArray = newArray;
    }

    valueStack = newValueStack;
    newValueStack.AfterCopyCleanup();

    return true;
}
#endif

void FilePathMap::MergeSort(std::vector<std::optional<Key>>& array, std::size_t leftIndex, std::size_t rightIndex)
{
    assert(array.at(rightIndex).has_value()); // Make sure you give a valid range!
    if (leftIndex >= rightIndex)
    {
        return;
    }
    std::size_t middleIndex = (leftIndex + rightIndex) / 2;
    MergeSort(array, leftIndex, middleIndex);
    MergeSort(array, middleIndex + 1, rightIndex);
    Merge(array, leftIndex, middleIndex, rightIndex);
}

void FilePathMap::Merge(std::vector<std::optional<Key>>& array, std::size_t leftIndex, std::size_t middleIndex, std::size_t rightIndex)
{
    std::size_t nLeft = middleIndex - leftIndex + 1;
    std::size_t nRight = rightIndex - middleIndex;

    std::optional<Key> left[nLeft];
    std::optional<Key> right[nRight];

    for (std::size_t i = 0; i < nLeft; ++i)
    {
        left[i].emplace(array[leftIndex + i].value());
    }
    for (std::size_t i = 0; i < nRight; ++i)
    {
        right[i].emplace(array[rightIndex + i].value());
    }

    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t k = leftIndex;

    while (i < nLeft && j < nRight)
    {
        if (left[i].value().hashID <= right[j].value().hashID)
        {
            array[k].value() = left[i].value();
            ++i;
        }
        else
        {
            array[k].value() = right[j].value();
            ++j;
        }
        ++k;
    }

    while (i < nLeft)
    {
        array[k].value() = left[i].value();
        ++i;
        ++k;
    }
    while (j < nRight)
    {
        array[k].value() = right[j].value();
        ++j;
        ++k;
    }
}

void FilePathMap::MergeSortKeyArray()
{
    MergeSort(keyArray, 0, keyArrayHasValueSize - 1);
}

void FilePathMap::InsertionSortKeyArray()
{
    const std::size_t range = keyArrayHasValueSize - 1;

    for (std::size_t index = 1; index <= range; ++index)
    {
        auto key = keyArray[index];
        std::size_t j = index - 1;

        while (j > 0 && keyArray.at(j).value().hashID > key.value().hashID)
        {
            keyArray.at(j + 1).value() = keyArray.at(j).value();
            --j;
        }
        keyArray.at(j + 1) = key;
    }
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
    keyArrayHasValueSize = 0;

    if (stringLengthHint_arg.has_value())
    {
        valueStack.Setup(numberOfElements_arg * sizeof(char[stringLengthHint_arg.value()]));
    }
    else
    {
        // Guess the average string size as 64 Characters to allocate ample space
        valueStack.Setup(numberOfElements_arg * sizeof(char[64]));
    }
}

void FilePathMap::Cleanup()
{
    valueStack.Cleanup();
    keyArray.clear();

    setupStatus = SetupStatus::NOT_BEGUN;
}

FilePathMap::FilePathMap()
    : keyArrayHasValueSize(0), keyArray(0)
{
}