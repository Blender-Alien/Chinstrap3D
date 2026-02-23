#include "MemoryTest.h"

#include "chinstrap/src/memory/Array.h"
#include "chinstrap/src/memory/StackAllocator.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

void TestStackArray2D()
{
    using namespace Chinstrap::Memory;

    constexpr uint32_t sizeFirstOrder = 2;
    constexpr uint32_t sizeSecondOrder = 2;

    StackAllocator allocator2;
    allocator2.Setup(sizeof(TestStruct[sizeFirstOrder][sizeSecondOrder]));

    StackArray2D<TestStruct> structArray(allocator2);

    assert(structArray.Allocate(sizeFirstOrder, sizeSecondOrder));

    assert(structArray.capacity() == sizeFirstOrder * sizeSecondOrder);

    for (int firstOrder = 0; firstOrder < structArray.firstOrderCapacity(); ++firstOrder)
    {
        for (int secondOrder = 0; secondOrder < structArray.secondOrderCapacity(); ++secondOrder)
        {
            assert(secondOrder < structArray.secondOrderCapacity() && firstOrder < structArray.firstOrderCapacity());

            *structArray.ptrAt(firstOrder, secondOrder) = TestStruct();
            structArray.ptrAt(firstOrder, secondOrder)->number2 = firstOrder + secondOrder;
        }
    }

    for (int firstOrder = 0; firstOrder < structArray.firstOrderCapacity(); ++firstOrder)
    {
        for (int secondOrder = 0; secondOrder < structArray.secondOrderCapacity(); ++secondOrder)
        {
            assert((structArray.ptrAt(firstOrder, secondOrder))->number2 == firstOrder + secondOrder);
        }
    }

    allocator2.Cleanup();
}

void TestStackArray()
{
    using namespace Chinstrap::Memory;

    StackAllocator allocator1;

    constexpr uint32_t size = 2;

    allocator1.Setup(sizeof(TestStruct[size]));

    StackArray<TestStruct> structArray(allocator1);

    assert(structArray.Allocate(size));

    assert(structArray.capacity() == size);

    for (int index = 0; index < structArray.capacity(); ++index)
    {
        assert(index < size);

        *structArray.ptrAt(index) = TestStruct();
        structArray.ptrAt(index)->hello = true;
        structArray.ptrAt(index)->number = 32.0f;
        structArray.ptrAt(index)->number2 = index;
    }

    for (int index = 0; index < structArray.capacity(); ++index)
    {
        assert(structArray.ptrAt(index)->hello);
        assert(structArray.ptrAt(index)->number == 32.0f);
        assert(structArray.ptrAt(index)->number2 == index);
    }

    allocator1.Cleanup();
}
