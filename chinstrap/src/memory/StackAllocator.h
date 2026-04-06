#pragma once

namespace Chinstrap::Memory
{
    // Custom stack that lives in heap memory
    struct StackAllocator
    {
        // Get pointer to the element at the top of the stack
        [[nodiscard]] std::byte* GetStackPointer() const
        {
            return GetHeapData()->hStackPointer;
        }

        [[nodiscard]] std::byte* DirectAllocate(uint32_t sizeInBytes_arg, std::size_t aligment_arg);

        // Roll back the entire stack
        void ClearStack();

        void Setup(uint32_t stackSizeInBytes_arg);
        void Cleanup();

        /**
         * This is useful if you want to get rid of a StackArray object, but have copied it before.
         */
        void CleanupKeepData();

        explicit StackAllocator() = default;
        ~StackAllocator();

        [[nodiscard]] uint64_t GetStackSizeInBytes() const { return GetHeapData()->hStackSizeInBytes; }
    private:

        // We store extra data on the heap right before the actual "stack"
        // so that our actual stack object can be only sizeof(std::byte*) for our basePointer.
        struct HeapStoredData
        {
            std::byte* hStackPointer = nullptr;
            std::byte* hTopPointer = nullptr;
            uint64_t hStackSizeInBytes = 0;
        };
        // This works because we know that we have one HeapStoredData object at the beginning of our allocated memory.
        [[nodiscard]] HeapStoredData* GetHeapData() const { return reinterpret_cast<HeapStoredData*>(basePointer); }

        std::byte* basePointer = nullptr;
    };
}
