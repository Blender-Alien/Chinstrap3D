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

    StackArray2D<sizeof(TestStruct)> structArray(allocator2);

    assert(structArray.Allocate(sizeFirstOrder, sizeSecondOrder));

    assert(structArray.capacity() == sizeFirstOrder * sizeSecondOrder);

    for (int firstOrder = 0; firstOrder < structArray.firstOrderCapacity(); ++firstOrder)
    {
        for (int secondOrder = 0; secondOrder < structArray.secondOrderCapacity(); ++secondOrder)
        {
            assert(secondOrder < structArray.secondOrderCapacity() && firstOrder < structArray.firstOrderCapacity());

            *(TestStruct*)structArray.at(firstOrder, secondOrder) = TestStruct();
            reinterpret_cast<TestStruct*>(structArray.at(firstOrder, secondOrder))->number2 = firstOrder + secondOrder;
        }
    }

    for (int firstOrder = 0; firstOrder < structArray.firstOrderCapacity(); ++firstOrder)
    {
        for (int secondOrder = 0; secondOrder < structArray.secondOrderCapacity(); ++secondOrder)
        {
            assert(reinterpret_cast<TestStruct*>(structArray.at(firstOrder, secondOrder))->number2 == firstOrder + secondOrder);
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

    StackArray<sizeof(TestStruct)> structArray(allocator1);

    assert(structArray.Allocate(size));

    assert(structArray.capacity() == size);

    for (int index = 0; index < structArray.capacity(); ++index)
    {
        assert(index < size);

        *reinterpret_cast<TestStruct*>(structArray.at(index)) = TestStruct();
        reinterpret_cast<TestStruct*>(structArray.at(index))->hello = true;
        reinterpret_cast<TestStruct*>(structArray.at(index))->number = 32.0f;
        reinterpret_cast<TestStruct*>(structArray.at(index))->number2 = index;
    }

    for (int index = 0; index < structArray.capacity(); ++index)
    {
        assert(reinterpret_cast<TestStruct*>(structArray.at(index))->hello);
        assert(reinterpret_cast<TestStruct*>(structArray.at(index))->number == 32.0f);
        assert(reinterpret_cast<TestStruct*>(structArray.at(index))->number2 == index);
    }

    allocator1.Cleanup();
}
