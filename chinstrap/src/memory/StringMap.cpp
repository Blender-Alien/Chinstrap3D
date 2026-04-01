#include "StringMap.h"

#include "../ops/Logging.h"

#include <climits>

#include "../Application.h"

std::unique_ptr<char> Chinstrap::Memory::ConvertToOSPath(const std::string_view& virtualFilePath)
{
    using namespace Chinstrap::Application;
    assert(App::programPath != nullptr);

    char* OSPath = new char[App::programPathRootIndex + virtualFilePath.length() + 1]; // + 1 for '\0'

    // On linux our full path looks something like this:
    // "/home/username/projects/chinstrap3d/app/res/shaders/Basic.frag"
    // A Windows example:
    // "C:\Projects\chinstrap3d\app\res\shaders\Basic.frag"
    //
    // The common theme is our project root directory "chinstrap3D" which can also
    // be a repository root for whatever game we're currently making like:
    // "/home/username/projects/MyGameRepo/MyGame/res/shaders/Basic.frag"
    //
    // We require that the virtual filepath looks like this:
    // "MyGame/res/shaders/Basic.frag", so we only have to go from executable location
    // to the root project directory.
    // We know that we're using Ninja Multi Config so our binary is going to be here:
    // "MyGameRepo/bin/MyGame/Release/MyGame", so we just need to go back 4 directories.

    strncpy(OSPath, App::programPath->data(), App::programPathRootIndex);

    strcpy(&OSPath[App::programPathRootIndex], virtualFilePath.data());

    std::unique_ptr<char> ptr(OSPath);

    return std::move(ptr);
}

using namespace Chinstrap::Memory;

StringMap::InsertRet StringMap::Insert(DevString& string_arg, const std::string_view& inputString_arg)
{
    if (setupStatus == SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("Insert was used on a FilePathMap, but Setup was not begun!");
        return InsertRet::BAD_REQUEST;
    }

    const DevString::HashIDType argHashID = std::hash<std::string_view>()(inputString_arg);

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
            string_arg.hashID.emplace(argHashID);

            ++keyArrayHasValueSize;
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

[[nodiscard]] std::optional<std::string_view> StringMap::Lookup(const DevString& key_arg) const
{
    if (setupStatus != SetupStatus::SETUP_DONE)
    {
        CHIN_LOG_WARN("A lookup in a StringMap was requested, before Setup was done!");
        return std::nullopt;
    }
    if (!key_arg.GetHashID().has_value())
    {
        CHIN_LOG_WARN("A lookup in a StringMap was requested, string has no hashID!");
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
    CHIN_LOG_WARN("StringMap lookup failed!");
    return std::nullopt;
}

bool StringMap::GrowBy(const uint32_t byNumberOfElements_arg, std::optional<uint32_t> avgStringLengthHint_arg)
{
    assert(byNumberOfElements_arg != 0);

    keyArray.resize(byNumberOfElements_arg + keyArray.capacity());

    StackAllocator newValueStack;
    if (avgStringLengthHint_arg.has_value())
    {
        const uint64_t growSize = valueStack.stackSizeInBytes + (sizeof(char[avgStringLengthHint_arg.value()]) * byNumberOfElements_arg);
        assert(growSize + valueStack.stackSizeInBytes);
        newValueStack.Setup(growSize);
    }
    else
    {
        const uint32_t previousElementValueSize = valueStack.stackSizeInBytes / (keyArray.capacity() - byNumberOfElements_arg);
        newValueStack.Setup(valueStack.stackSizeInBytes + (byNumberOfElements_arg * previousElementValueSize));
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

std::optional<std::string_view> StringMap::Iterate(uint32_t index) const
{
    if (keyArray.at(index).has_value())
    {
        return keyArray.at(index).value().charArray.dataConst();
    }
    return std::nullopt;
}

void StringMap::MergeSort(std::vector<std::optional<Key>>& array, std::size_t leftIndex, std::size_t rightIndex)
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

void StringMap::Merge(std::vector<std::optional<Key>>& array, std::size_t leftIndex, std::size_t middleIndex, std::size_t rightIndex)
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

void StringMap::MergeSortKeyArray()
{
    if (keyArrayHasValueSize == 0 || keyArrayHasValueSize == 1)
    {
        return;
    }
    MergeSort(keyArray, 0, keyArrayHasValueSize - 1);
}

void StringMap::InsertionSortKeyArray()
{
    if (keyArrayHasValueSize == 0 || keyArrayHasValueSize == 1)
    {
        return;
    }

    for (std::size_t index = 1; index < keyArrayHasValueSize; ++index)
    {
        std::size_t j = index;

        while (j > 0 && keyArray.at(j-1).value().hashID > keyArray.at(j).value().hashID)
        {
            std::swap(keyArray.at(j).value(), keyArray.at(j-1).value());
            --j
            ;
        }
    }
}

void StringMap::Setup(uint32_t numberOfElements, uint32_t totalValuesSize)
{
    if (setupStatus != SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("A FilePathMap was ordered to Setup(), but it already was or was in the process!");
        assert(false); // We have already setup!
        return;
    }
    setupStatus = SetupStatus::IN_SETUP;

    keyArray.resize(numberOfElements);
    keyArrayHasValueSize = 0;

    valueStack.Setup(numberOfElements * sizeof(char[totalValuesSize]));
}

void StringMap::Cleanup()
{
    // TODO: Serialize numberOfElements and totalValueSize for next run

    valueStack.Cleanup();
    keyArray.clear();

    setupStatus = SetupStatus::NOT_BEGUN;
}

StringMap::StringMap()
    : keyArrayHasValueSize(0), keyArray(0)
{
}