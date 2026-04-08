#pragma once

#include "StackAllocator.h"
#include "StackArray.h"
#include "MemoryPool.h"

#include <optional>

namespace Chinstrap::Memory
{
    enum class HashInsertRet
    {
        SUCCESS, COLLISION_OR_DUPLICATE,
        NO_KEY_CAPACITY, NO_VALUE_CAPACITY, BAD_REQUEST
    };

    /**
     * Compact hash map
     * @tparam key Type to hash, has to implement std::hash()
     * @tparam value Type to store
     */
    template<typename key, typename value>
    struct CompHashMap;
}

template<typename key, typename value>
struct Chinstrap::Memory::CompHashMap
{
    typedef std::size_t hashIDType;

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

    void Setup(uint32_t numberOfElements);
    void EndSetup();

    void Cleanup();

    explicit CompHashMap() = default;

private:
    enum class SetupStatus { NOT_BEGUN, IN_SETUP, SETUP_DONE };

    SetupStatus setupStatus = SetupStatus::NOT_BEGUN;

    MemoryPool<value> valuePool;

    struct KeyContainer
    {
        hashIDType hashID;
        value* valuePointer;
    };
    std::vector<std::optional<KeyContainer>> keyArray;
    std::size_t keyArrayHasValueSize = 0;
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
                    return HashInsertRet::NO_VALUE_CAPACITY;
                }
                memcpy(keyIndex.value().valuePointer, &value_arg, sizeof(value));

                ++keyArrayHasValueSize;
                if (setupStatus == SetupStatus::SETUP_DONE)
                {
                    // TODO
                    //InsertionSortKeyArray();
                } // else, we will sort later in EndSetup()

                return HashInsertRet::SUCCESS;
            }
            if (keyIndex.value().hashID == keyHashID) { return HashInsertRet::COLLISION_OR_DUPLICATE; }
        }
        return HashInsertRet::NO_KEY_CAPACITY;
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
        // TODO
        //MergeSortKeyArray();
        setupStatus = SetupStatus::SETUP_DONE;
    }

    template <typename key, typename value>
    void CompHashMap<key, value>::Cleanup()
    {
        valuePool.Cleanup();
        keyArray.clear();
        setupStatus = SetupStatus::NOT_BEGUN;
    }
}
