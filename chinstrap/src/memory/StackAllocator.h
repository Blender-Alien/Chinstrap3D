#pragma once

#include "../ops/Logging.h"

namespace Chinstrap::Memory
{
    struct StackAllocator
    {
        typedef uint32_t* StackPointer;

        // Get pointer to the element at the top of the stack
        template <typename type>
        [[nodiscard]] type* GetStackPointer() const
        {
            return static_cast<type*>(static_cast<void*>(stackPointer));
        }

        template<typename type>
        [[nodiscard]] type* Allocate(const uint32_t count)
        {
            uint32_t sizeInBytes = sizeof(type) * count;
            if (topPointer + sizeInBytes > basePointer + stackSizeInBytes) [[unlikely]]
            {
                CHIN_LOG_ERROR("Stack allocator out of memory!");
                return nullptr;
            }
            topPointer += sizeInBytes;
            stackPointer = topPointer - sizeInBytes;

            return static_cast<type*>(static_cast<void*>(stackPointer));
        }

        // Roll back the entire stack
        void ClearStack();

        explicit StackAllocator(const uint32_t stackSizeInBytes_arg);
        ~StackAllocator();
    private:

        uint32_t stackSizeInBytes;
        uint32_t* basePointer;
        uint32_t* topPointer;
        StackPointer stackPointer;
    };

}
