#include "FilePathMap.h"

#include "../ops/Logging.h"

#include <climits>

#if __linux__
#include <unistd.h>
#elif _WIN64
#include <windows.h>
#endif

using namespace Chinstrap::Memory;

char* FilePath::ConvertToOSPath(const std::string_view& virtualFilePath)
{
    assert(FilePathMap::programPath != nullptr);

    char* OSPath = new char[FilePathMap::rootIndex + virtualFilePath.length() + 1]; // + 1 for '\0'

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

    strncpy(OSPath, FilePathMap::programPath->data(), FilePathMap::rootIndex);

    strcpy(&OSPath[FilePathMap::rootIndex], virtualFilePath.data());

    return OSPath;
}

FilePathMap::InsertRet FilePathMap::Insert(FilePath& filepath_arg, const std::string_view& inputString_arg)
{
    if (setupStatus == SetupStatus::NOT_BEGUN)
    {
        CHIN_LOG_WARN("Insert was used on a FilePathMap, but Setup was not begun!");
        return InsertRet::BAD_REQUEST;
    }

    const FilePath::HashID argHashID = std::hash<std::string_view>()(inputString_arg);

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
    if (!key_arg.GetHashID().has_value())
    {
        CHIN_LOG_WARN("A lookup in a FilePathMap was requested, filePath has no hashID!");
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
bool FilePathMap::GrowBy(uint32_t byNumberOfElements_arg, std::optional<uint32_t> stringLengthHint_arg)
{
    assert(byNumberOfElements_arg != 0);

    keyArray.resize(byNumberOfElements_arg + keyArray.capacity());

    StackAllocator newValueStack;
    if (stringLengthHint_arg.has_value())
    {
        newValueStack.Setup((byNumberOfElements_arg + keyArray.capacity()) * sizeof(char[stringLengthHint_arg.value()]));
    }
    else
    {
        // Guess the average string size as 64 Characters to allocate ample space
        newValueStack.Setup((byNumberOfElements_arg + keyArray.capacity()) * sizeof(char[64]));
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

std::optional<std::string_view> FilePathMap::Iterate(uint32_t index) const
{
    if (keyArray.at(index).has_value())
    {
        return keyArray.at(index).value().charArray.dataConst();
    }
    return std::nullopt;
}

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
    if (keyArrayHasValueSize == 0 || keyArrayHasValueSize == 1)
    {
        return;
    }
    MergeSort(keyArray, 0, keyArrayHasValueSize - 1);
}

void FilePathMap::InsertionSortKeyArray()
{
    if (keyArrayHasValueSize == 0 || keyArrayHasValueSize == 1)
    {
        return;
    }
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
    { // OSPath has to be absolute, so that we can execute the game binary form anywhere and still load our resources
#if __linux__
        char result[PATH_MAX];
        const auto count = readlink("/proc/self/exe", result, PATH_MAX);
        programPath = new std::string(result, (count > 0) ? count : 0);
        constexpr char slash = '/';
#elif _WIN64
        char result[MAX_PATH];
        programPath = new std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
        constexpr char slash = '\\';
#endif
        std::vector<std::size_t> slashPositions;
        { // Find all slashes
            slashPositions.reserve(8); // Guess an initial number to avoid reallocations
            std::size_t position = 0;

            while ((position = programPath->find(slash, position)) != std::string::npos)
            {
                slashPositions.push_back(position);
                ++position;
            }
        }
        rootIndex = slashPositions.at(slashPositions.size() - 4) + 1;
    }

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

    delete programPath;

    setupStatus = SetupStatus::NOT_BEGUN;
}

FilePathMap::FilePathMap()
    : keyArrayHasValueSize(0), keyArray(0)
{
}