#pragma once

namespace Chinstrap::Memory
{

    // Custom stack that lives in heap memory
    struct StackAllocator
    {
        [[nodiscard]] std::byte* DirectAllocate(uint32_t sizeInBytes_arg, std::size_t alignment_arg);

        // Roll back the entire stack
        void ClearStack();

        void Setup(uint32_t stackSizeInBytes_arg);
        void Cleanup();

        /**
         * This is useful if you want to get rid of this StackArray object, but have copied it before.
         */
        void CleanupKeepData();

        explicit StackAllocator() = default;
        ~StackAllocator();

        uint64_t stackSizeInBytes = 0;
    private:
        std::byte* basePointer = nullptr;

        // Note that you could store this on the heap directly below the stack, but that would potentially
        // start to incur extra cash misses when the stack grows large enough.
        // That is not worth a reduced size of StackAllocator at this time.
        std::byte* stackPointer = nullptr;
        std::byte* topPointer = nullptr;
    };
}