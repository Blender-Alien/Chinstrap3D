#pragma once

#include "StackAllocator.h"

namespace Chinstrap::Memory
{
    // Array that lives in heap memory using a Chinstrap::Memory::StackAllocator
    template<uint32_t elementSize>
    struct StackArray
    {
        explicit StackArray(StackAllocator& stackAllocator_arg)
            : stackAllocator(stackAllocator_arg)
        {}

        [[nodiscard]] bool Allocate(const uint32_t arrayCapacity_arg)
        {
            arrayCapacity = arrayCapacity_arg;

            basePointer = stackAllocator.DirectAllocate(elementSize * arrayCapacity);
            if (basePointer == nullptr)
                return false;

            return true;
        }

        std::byte* at(const uint32_t index)
        {
            assert(index < arrayCapacity);
            return (basePointer + index * elementSize);
        }

        void* data()
        {
            return basePointer;
        }
        std::byte* lastElement()
        {
            return (basePointer + (arrayCapacity-1) * elementSize);
        }

        [[nodiscard]] uint32_t capacity() const
        {
            assert(arrayCapacity > 0);
            return arrayCapacity;
        }

    private:
        std::byte* basePointer = nullptr;
        uint32_t arrayCapacity = 0;

        StackAllocator& stackAllocator;
    };

    // 2Dimensional array that lives in heap memory using a Chinstrap::Memory::StackAllocator
    template<uint32_t elementSize>
    struct StackArray2D
    {
        explicit StackArray2D(StackAllocator& stackAllocator_arg)
            : stackAllocator(stackAllocator_arg) {}

        [[nodiscard]] bool Allocate(const uint32_t capacityFirstOrder_arg, const uint32_t capacitySecondOrder_arg)
        {
            capacityFirstOrder = capacityFirstOrder_arg;
            capacitySecondOrder = capacitySecondOrder_arg;

            basePointer = stackAllocator.DirectAllocate(capacitySecondOrder * capacityFirstOrder * elementSize);
            if (basePointer == nullptr)
                return false;

            return true;
        }

        std::byte* at(const uint32_t firstOrderIndex, const uint32_t secondOrderIndex)
        {
            assert((firstOrderIndex * capacitySecondOrder + secondOrderIndex) < capacityFirstOrder * capacitySecondOrder);
            return (basePointer + firstOrderIndex * capacitySecondOrder * elementSize + secondOrderIndex * elementSize);
        }

        [[nodiscard]] uint32_t capacity() const
        {
            return capacityFirstOrder * capacitySecondOrder;
        }

        [[nodiscard]] uint32_t firstOrderCapacity() const
        {
            assert(capacityFirstOrder > 0);
            return capacityFirstOrder;
        }

        [[nodiscard]] uint32_t secondOrderCapacity() const
        {
            assert(capacitySecondOrder > 0);
            return capacitySecondOrder;
        }

    private:
        std::byte* basePointer = nullptr;

        uint32_t capacityFirstOrder = 0;
        uint32_t capacitySecondOrder = 0;

        StackAllocator& stackAllocator;
    };
}

