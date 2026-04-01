#pragma once

namespace Chinstrap::Memory
{
    // Custom stack that lives in heap memory
    struct StackAllocator
    {
        typedef std::byte* StackPointer;

        // Get pointer to the element at the top of the stack
        [[nodiscard]] std::byte* GetStackPointer() const
        {
            return stackPointer;
        }

        [[nodiscard]] std::byte* DirectAllocate(uint32_t sizeInBytes_arg);

        // Roll back the entire stack
        void ClearStack();

        void Setup(uint32_t stackSizeInBytes_arg);
        void Cleanup();
        // Do this if you have copied this stackAllocator to another one
        void AfterCopyCleanup();

        explicit StackAllocator() = default;
        ~StackAllocator();


        uint32_t stackSizeInBytes = 0;
    private:

        std::byte* basePointer = nullptr;
        std::byte* topPointer = nullptr;
        StackPointer stackPointer = nullptr;
    };
}
