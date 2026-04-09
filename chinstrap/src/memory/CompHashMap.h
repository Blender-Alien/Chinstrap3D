#pragma once

#include "MemoryPool.h"

#include <optional>

namespace Chinstrap::Memory
{
    enum class HashInsertRet
    {
        SUCCESS, COLLISION_OR_DUPLICATE,
        NO_CAPACITY,
        BAD_REQUEST
    };

    /**
     * Compact hash map
     * @tparam key Type to hash, has to have template specialization std::hash<key>::operator()
     * @tparam value Type to store
     */
    template<typename key, typename value>
    struct CompHashMap;
}

template<typename key, typename value>
struct Chinstrap::Memory::CompHashMap
{
    typedef std::size_t hashIDType;
    struct KeyContainer
    {
        hashIDType hashID;
        value* valuePointer;
    };

    [[nodiscard]] HashInsertRet Insert(const key& key_arg, const value& value_arg);

    /**
     * TODO: This is not thread safe!
     * @return Pointer with current address of value. CAN BE INVALIDATED DO NOT SAVE!
     */
    [[nodiscard]] std::optional<value*> Lookup(const key& key_arg) const;

    /**
     * TODO: This is not thread safe!
     * @return Pointer with current address of value. CAN BE INVALIDATED DO NOT SAVE!
     */
    [[nodiscard]] std::optional<value*> Iterate(uint32_t index) const;


    /**
     * @param numberOfElements_arg How many more elements should fit?
     */
    void GrowBy(uint32_t numberOfElements_arg);

    void Setup(uint32_t numberOfElements);
    void EndSetup();

    void Cleanup();

    explicit CompHashMap() = default;

    std::size_t keyArrayHasValueSize = 0;
private:
    void InsertionSortKeyArray();

    void MergeSortKeyArray();
    void MergeSort(std::vector<std::optional<KeyContainer>>& array, std::size_t leftIndex, std::size_t rightIndex);
    void Merge(std::vector<std::optional<KeyContainer>>& array, std::size_t leftIndex, std::size_t middleIndex, std::size_t rightIndex);

    enum class SetupStatus { NOT_BEGUN, IN_SETUP, SETUP_DONE };

    SetupStatus setupStatus = SetupStatus::NOT_BEGUN;

    MemoryPool<value> valuePool;
    // TODO: We could like make this a stackArray really, would save lots of stack-object size.
    std::vector<std::optional<KeyContainer>> keyArray;
};

namespace Chinstrap::Memory
{
    template <typename key, typename value>
    HashInsertRet CompHashMap<key, value>::Insert(
        const key& key_arg, const value& value_arg)
    {
        if (setupStatus == SetupStatus::NOT_BEGUN)
        {
            CHIN_LOG_WARN("Insert was used on a CompHashMap, but Setup was not begun!");
            return HashInsertRet::BAD_REQUEST;
        }

        const hashIDType keyHashID = std::hash<key>()(key_arg);

        for (auto& keyIndex : keyArray)
        {
            if (!keyIndex.has_value())
            {
                keyIndex.emplace();
                keyIndex.value().hashID = keyHashID;

                if (keyIndex.value().valuePointer = valuePool.Allocate();
                    keyIndex.value().valuePointer == nullptr)
                {
                    keyIndex.reset();
                    return HashInsertRet::NO_CAPACITY;
                }
                memcpy(keyIndex.value().valuePointer, &value_arg, sizeof(value));

                ++keyArrayHasValueSize;
                if (setupStatus == SetupStatus::SETUP_DONE)
                {
                    InsertionSortKeyArray();
                } // else, we will sort later in EndSetup()

                return HashInsertRet::SUCCESS;
            }
            if (keyIndex.value().hashID == keyHashID) { return HashInsertRet::COLLISION_OR_DUPLICATE; }
        }
        return HashInsertRet::NO_CAPACITY;
    }

    template <typename key, typename value>
    std::optional<value*> CompHashMap<key, value>::Lookup(const key& key_arg) const
    {
        if (setupStatus != SetupStatus::SETUP_DONE)
        {
            CHIN_LOG_WARN("A lookup in a CompHashMap was requested, before Setup was done!");
            return std::nullopt;
        }
        const hashIDType keyHashID = std::hash<key>()(key_arg);
        { // Binary Search
            uint32_t left = 0;
            uint32_t right = keyArray.size() - 1;
            while (left <= right)
            {
                uint32_t middle = (left + right) / 2;

                if (keyArray.at(middle).has_value()
                    && keyArray.at(middle).value().hashID == keyHashID)
                {
                    return keyArray.at(middle).value().valuePointer;
                }
                else if (keyArray.at(middle).has_value()
                    && keyArray.at(middle).value().hashID < keyHashID)
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

    template <typename key, typename value>
    std::optional<value*> CompHashMap<key, value>::Iterate(const uint32_t index) const
    {
        if (keyArray.at(index).has_value())
        {
            return keyArray.at(index).value().valuePointer;
        }
        return std::nullopt;
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::GrowBy(uint32_t numberOfElements_arg)
    {
        assert(numberOfElements_arg != 0);

        keyArray.resize(numberOfElements_arg + keyArray.capacity());

        MemoryPool<value> newPool;
        auto ret = newPool.Setup(numberOfElements_arg + keyArray.capacity());

        for (auto& keyIndex : keyArray)
        {
            if (!keyIndex.has_value())
                continue;

            auto ptr = newPool.Allocate();
            ENSURE_OR_RETURN((ptr != nullptr));

            memcpy(ptr, keyIndex.value().valuePointer, sizeof(value));

            keyIndex.value().valuePointer = ptr;
        }

        valuePool = newPool;
        newPool.CleanupKeepData();
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::Setup(uint32_t numberOfElements)
    {
        ENSURE_OR_RETURN((setupStatus == SetupStatus::NOT_BEGUN));
        setupStatus = SetupStatus::IN_SETUP;

        keyArray.resize(numberOfElements);
        keyArrayHasValueSize = 0;

        auto ret = valuePool.Setup(numberOfElements);
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::EndSetup()
    {
        MergeSortKeyArray();
        setupStatus = SetupStatus::SETUP_DONE;
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::Cleanup()
    {
        valuePool.Cleanup();
        keyArray.clear();
        setupStatus = SetupStatus::NOT_BEGUN;
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::InsertionSortKeyArray()
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
                --j;
            }
        }
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::MergeSortKeyArray()
    {
        if (keyArrayHasValueSize == 0 || keyArrayHasValueSize == 1)
        {
            return;
        }
        MergeSort(keyArray, 0, keyArrayHasValueSize - 1);
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::MergeSort(std::vector<std::optional<KeyContainer>>& array, std::size_t leftIndex,
        std::size_t rightIndex)
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

    template <typename key, typename value>
    void CompHashMap<key, value>::Merge(std::vector<std::optional<KeyContainer>>& array, std::size_t leftIndex,
        std::size_t middleIndex, std::size_t rightIndex)
    {
        std::size_t nLeft = middleIndex - leftIndex + 1;
        std::size_t nRight = rightIndex - middleIndex;

        std::optional<KeyContainer> left[nLeft];
        std::optional<KeyContainer> right[nRight];

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
}