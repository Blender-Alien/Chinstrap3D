#pragma once
#include "../ops/Logging.h"

namespace Chinstrap::Memory
{
    /* We keep a freePointer that points to the first free chunk IF there is one,
     * otherwise freePointer is equal to nullptr.
     * All free chunks contain one uint32_t (nextFreeIndex) that specifies the index of the next free chunk,
     * creating a linked list. That's why we require that the sizeof(uint32_t) <= sizeof(type).
     * If a free chunk is the last free chunk, it's uint32_t (nextFreeIndex) is equal to its own index.
     */

    // Continuous fixed size array of fixed size members that live in heap memory
    template<typename type>
    struct MemoryPool
    {
        typedef uint32_t nextFreeIndex;

        /**
         * Note that this DOES NOT call any constructors, do that if you after, if you wish!
         * @return type* Pointer to new pool element
         */
        [[nodiscard]] type* Allocate();

        /**
         * Note that this DOES NOT call any destructors, do that before, if you wish!
         * @param dataPtr
         */
        void Deallocate(type** dataPtr);

        [[nodiscard]] bool Setup(uint32_t numberOfElements_arg);

        void Cleanup()
        {
            free(basePointer);
            basePointer = nullptr;
        }
        void CleanupKeepData()
        {
            basePointer = nullptr;
        }

        explicit MemoryPool() = default;
        ~MemoryPool()
        {
            // Did we call Cleanup???
            assert(basePointer == nullptr);
        }

    private:
        std::byte* basePointer = nullptr;
        std::byte* freePointer = nullptr;
    };

/* These macros get undefined at the bottom of this file.
 * The reason for not making them functions is, that I'm worried about lots of
 * extra function calls just so for the sake of readability. We actually care about performance!
 * The alternative is to manually inline all off this stuff, but I like how
 * these macros help by defining simple and reasonable things we might want to do,
 * and then integrating them into the complexer logic of this data type.
 */
#define nextFreeIndex_at_freePtr    reinterpret_cast<nextFreeIndex*>(freePointer)
#define nextFreeIndex_at(index)     reinterpret_cast<nextFreeIndex*>(data_at(index))
#define type_at(index)              reinterpret_cast<type*>(data_at(index))
#define data_at(index)              (basePointer + index * sizeof(type))

    template <typename type>
    type* MemoryPool<type>::Allocate()
    {
        if (basePointer == nullptr || freePointer == nullptr)
        {
            return nullptr;
        }
        auto allocatedPointer = reinterpret_cast<type*>(freePointer);

        if (*nextFreeIndex_at_freePtr == (allocatedPointer - type_at(0)))
        { // If the nextFreeIndex is equal to index of the chunk, it is the last free element
            freePointer = nullptr;
            return allocatedPointer;
        }

        // Set freeBasePointer to next free chunk by getting the next free index at the current chunk
        freePointer = data_at(*nextFreeIndex_at_freePtr);
        return allocatedPointer;
    }

    template <typename type>
    void MemoryPool<type>::Deallocate(type** dataPtr)
    {
        ENSURE_OR_RETURN((*dataPtr != nullptr));
        auto freedData = reinterpret_cast<std::byte*>(*dataPtr);

        assert(basePointer != nullptr);

        if (freePointer == nullptr)
        { // This is the first free chunk, so set the nextFreeIndex to its own index
            *reinterpret_cast<nextFreeIndex*>(freedData) = reinterpret_cast<type*>(freedData) - type_at(0);
        }
        else
        { // Set nextFreeIndex to previous beginning of free chunk list
            *freedData = *(freePointer + sizeof(type) * (*nextFreeIndex_at_freePtr));
        }

        freePointer = freedData;
        *dataPtr = nullptr;
    }

    template <typename type>
    bool MemoryPool<type>::Setup(uint32_t numberOfElements_arg)
    {
        assert(sizeof(uint32_t) <= sizeof(type)); // We need this in order to store the "next-free-index"

        assert(numberOfElements_arg > 1); // We should not make a memory pool for only one element

        basePointer = static_cast<std::byte*>(malloc(sizeof(type) * numberOfElements_arg));
        if (basePointer == nullptr)
        {
            return false;
        }
        freePointer = data_at(0);

        // Set all free chunks of memory to include the index of the next free chunk of memory
        for (uint32_t index = 0; index < numberOfElements_arg; ++index)
        {
            if (index == numberOfElements_arg-1)
            { // The last free element defines its own index as nextFreeIndex
                *nextFreeIndex_at(index) = index;
                break;
            }
            *nextFreeIndex_at(index) = index + 1;
        }
        return true;
    }
}

#undef nextFreeIndex_at_freePtr
#undef nextFreeIndex_at
#undef type_at
#undef data_at