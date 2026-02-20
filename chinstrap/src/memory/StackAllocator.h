#pragma once

#include "../ops/Logging.h"

namespace Chinstrap::Memory
{
    // Custom stack that lives in heap memory
    struct StackAllocator
    {
        typedef std::byte* StackPointer;

        // Get pointer to the element at the top of the stack
        template <typename type>
        [[nodiscard]] type* GetStackPointer() const
        {
            return static_cast<type*>(static_cast<void*>(stackPointer));
        }

        [[nodiscard]] std::byte* DirectAllocate(uint32_t sizeInBytes_arg);

        // Roll back the entire stack
        void ClearStack();

        void Setup(uint32_t stackSizeInBytes_arg);
        void Cleanup();

        explicit StackAllocator() = default;
        ~StackAllocator();
    private:

        uint32_t stackSizeInBytes = 0;
        std::byte* basePointer = nullptr;
        std::byte* topPointer = nullptr;
        StackPointer stackPointer = nullptr;
    };
}
