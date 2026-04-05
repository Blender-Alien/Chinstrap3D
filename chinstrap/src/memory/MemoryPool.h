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

    // TODO: Let's clean this mess up a bit, we should also call destructors on non-trivial types

    // Continuous fixed size array of fixed size members that live in heap memory
    template<typename type>
    struct MemoryPool
    {
        typedef uint32_t nextFreeIndex;

        [[nodiscard]] type* Allocate()
        {
            if (freePointer == nullptr)
            {
                return nullptr;
            }
            std::byte* allocatedPointer = freePointer;

            if (*reinterpret_cast<nextFreeIndex*>(freePointer) == (reinterpret_cast<type*>(allocatedPointer) - reinterpret_cast<type*>(basePointer)))
            { // If the nextFreeIndex is equal to index of the chunk, it is the last free element
                freePointer = nullptr;
                return reinterpret_cast<type*>(allocatedPointer);
            }

            // Set freeBasePointer to next free chunk by getting the next free index at the current chunk
            freePointer = basePointer + sizeof(type) * (*reinterpret_cast<nextFreeIndex*>(freePointer));
            return reinterpret_cast<type*>(allocatedPointer);
        }

        void Deallocate(type** dataPtr)
        {
            ENSURE_OR_RETURN((*dataPtr != nullptr));
            auto freedData = reinterpret_cast<std::byte*>(*dataPtr);

            if (freePointer == nullptr)
            { // This is the first free chunk, so set the nextFreeIndex to its own index
                *reinterpret_cast<nextFreeIndex*>(freedData) = reinterpret_cast<type*>(freedData) - reinterpret_cast<type*>(basePointer);
            }
            else
            { // Set nextFreeIndex to previous beginning of free chunk list
                *freedData = *(freePointer + sizeof(type) * (*reinterpret_cast<nextFreeIndex*>(freePointer)));
            }

            freePointer = freedData;
            *dataPtr = nullptr;
        }

        [[nodiscard]] bool Setup(uint32_t numberOfElements_arg)
        {
            assert(sizeof(uint32_t) <= sizeof(type)); // We need this in order to store the "next-free-index"

            assert(numberOfElements_arg > 1); // We should not make a memory pool for only one element

            basePointer = static_cast<std::byte*>(malloc(sizeof(type) * numberOfElements_arg));
            if (basePointer == nullptr)
            {
                return false;
            }
            freePointer = basePointer;

            // Set all free chunks of memory to include the index of the next free chunk of memory
            for (uint32_t index = 0; index < numberOfElements_arg; ++index)
            {
                if (index == numberOfElements_arg-1)
                { // The last free element defines its own index as nextFreeIndex
                    *reinterpret_cast<nextFreeIndex*>((basePointer + sizeof(type) * index)) = index;
                    break;
                }
                *reinterpret_cast<nextFreeIndex*>((basePointer + sizeof(type) * index)) = index + 1;
            }
            return true;
        }

        void Cleanup()
        {
            free(basePointer);
            basePointer = nullptr;
        }

        // Clear the entire pool
        void ClearPool()
        {
            freePointer = basePointer;
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
}
