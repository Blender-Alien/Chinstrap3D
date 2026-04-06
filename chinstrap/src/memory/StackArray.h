#pragma once

#include "StackAllocator.h"
#include "../ops/Logging.h"

#define CHIN_STACK_ARRAY_MEM_SIZE(expr) sizeof(expr) + 8 // 8 is the size of HeapStoredData for StackArray and StackArray2D

namespace Chinstrap::Memory
{
    // Array that lives in heap memory using a Chinstrap::Memory::StackAllocator
    template<typename type>
    struct StackArray
    {
        explicit StackArray(StackAllocator* stackAllocator_arg)
        {
            // As long as we are not allocated, we store our future StackAllocator in dataPointer
            dataPointer = reinterpret_cast<std::byte*>(stackAllocator_arg);
        }

        /**
         * We can only allocate once!
         */
        [[nodiscard]] bool Allocate(const uint32_t arrayCapacity_arg)
        {
            auto ptr = reinterpret_cast<StackAllocator*>(dataPointer)->DirectAllocate(sizeof(HeapStoredData)
                + sizeof(type) * arrayCapacity_arg, alignof(type));

            ENSURE_OR_RETURN_FALSE((ptr != nullptr));

            // We change dataPointer to point to our memory in StackAllocator instead of to our StackAllocator
            dataPointer = ptr;

            GetHeapData()->arrayCapacity = arrayCapacity_arg;

            return true;
        }

        type* ptrAt(const uint32_t index)
        {
            assert(index < GetHeapData()->arrayCapacity);
            return reinterpret_cast<type*>(GetBasePtr() + index * sizeof(type));
        }

        type* data()
        {
            return reinterpret_cast<type*>(GetBasePtr());
        }
        const type* dataConst() const
        {
            return reinterpret_cast<const type*>(GetBasePtr());
        }
        std::byte* lastElement()
        {
            return (GetBasePtr() + (GetHeapData()->arrayCapacity-1) * sizeof(type));
        }

        [[nodiscard]] uint32_t capacity() const
        {
            assert(GetHeapData()->arrayCapacity > 0);
            return GetHeapData()->arrayCapacity;
        }

        StackArray& operator=(const StackArray& stackArray_arg)
        {
            if (this == &stackArray_arg)
            {
                return *this;
            }
            dataPointer = stackArray_arg.dataPointer;
            return *this;
        }

    private:
        [[nodiscard]] std::byte* GetBasePtr() const { return dataPointer + sizeof(HeapStoredData); }

        struct HeapStoredData
        {
            uint32_t arrayCapacity = 0;
        };
        [[nodiscard]] HeapStoredData* GetHeapData() const { return reinterpret_cast<HeapStoredData*>(dataPointer);}

        std::byte* dataPointer = nullptr;
    };

    // 2Dimensional array that lives in heap memory using a Chinstrap::Memory::StackAllocator
    template<typename type>
    struct StackArray2D
    {
        explicit StackArray2D(StackAllocator* stackAllocator_arg)
        {
            // As long as we are not allocated, we store our future StackAllocator in dataPointer
            dataPointer = reinterpret_cast<std::byte*>(stackAllocator_arg);
        }

        [[nodiscard]] bool Allocate(const uint32_t capacityFirstOrder_arg, const uint32_t capacitySecondOrder_arg)
        {
            auto ptr = reinterpret_cast<StackAllocator*>(dataPointer)->DirectAllocate(sizeof(HeapStoredData)
                + capacitySecondOrder_arg * capacityFirstOrder_arg * sizeof(type), alignof(type));
            ENSURE_OR_RETURN_FALSE((ptr != nullptr));

            // We change dataPointer to point to our memory in StackAllocator instead of to our StackAllocator
            dataPointer = ptr;

            GetHeapData()->capacityFirstOrder = capacityFirstOrder_arg;
            GetHeapData()->capacitySecondOrder = capacitySecondOrder_arg;

            return true;
        }

        type* ptrAt(const uint32_t firstOrderIndex, const uint32_t secondOrderIndex)
        {
            assert((firstOrderIndex * GetHeapData()->capacitySecondOrder + secondOrderIndex) < GetHeapData()->capacityFirstOrder * GetHeapData()->capacitySecondOrder);
            return reinterpret_cast<type*>((GetBasePtr() + firstOrderIndex * GetHeapData()->capacitySecondOrder * sizeof(type) + secondOrderIndex * sizeof(type)));
        }

        [[nodiscard]] uint32_t capacity() const
        {
            return GetHeapData()->capacityFirstOrder * GetHeapData()->capacitySecondOrder;
        }

        [[nodiscard]] uint32_t firstOrderCapacity() const
        {
            assert(GetHeapData()->capacityFirstOrder > 0);
            return GetHeapData()->capacityFirstOrder;
        }

        [[nodiscard]] uint32_t secondOrderCapacity() const
        {
            assert(GetHeapData()->capacitySecondOrder > 0);
            return GetHeapData()->capacitySecondOrder;
        }

    private:
        [[nodiscard]] std::byte* GetBasePtr() const { return dataPointer + sizeof(HeapStoredData); }

        struct HeapStoredData
        {
            uint32_t capacityFirstOrder = 0;
            uint32_t capacitySecondOrder = 0;
        };
        [[nodiscard]] HeapStoredData* GetHeapData() const { return reinterpret_cast<HeapStoredData*>(dataPointer);}

        std::byte* dataPointer = nullptr;
    };
}

