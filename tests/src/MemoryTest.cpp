#include "MemoryTest.h"

#include "chinstrap/src/memory/CompHashMap.h"
#include "chinstrap/src/memory/MemoryPool.h"
#include "chinstrap/src/memory/StackArray.h"
#include "chinstrap/src/memory/StackAllocator.h"

void TestStackArray2D()
{
    using namespace Chinstrap::Memory;

    constexpr uint32_t sizeFirstOrder = 2;
    constexpr uint32_t sizeSecondOrder = 2;

    StackAllocator allocator2;
    allocator2.Setup(CHIN_STACK_ARRAY_MEM_SIZE(TestStruct[sizeFirstOrder][sizeSecondOrder]));

    StackArray2D<TestStruct> structArray(&allocator2);

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

    allocator1.Setup(CHIN_STACK_ARRAY_MEM_SIZE(TestStruct[size]));

    StackArray<TestStruct> structArray(&allocator1);

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

void TestMemoryPool()
{
    using namespace Chinstrap::Memory;

    MemoryPool<TestStruct> memoryPool;

    assert(memoryPool.Setup(3));

    TestStruct* struct1 = nullptr;
    TestStruct* struct2 = nullptr;
    TestStruct* struct3 = nullptr;

    // Allocate
    struct1 = memoryPool.Allocate();
    assert(struct1 != nullptr);
    struct2 = memoryPool.Allocate();
    assert(struct2 != nullptr);
    struct3 = memoryPool.Allocate();
    assert(struct3 != nullptr);
    assert(memoryPool.Allocate() == nullptr);

    // Deallocate and Allocate Again
    memoryPool.Deallocate(&struct1);
    assert(struct1 == nullptr);
    struct1 = memoryPool.Allocate();
    assert(struct1 != nullptr);

    // Deallocate randomly
    memoryPool.Deallocate(&struct1);
    assert(struct1 == nullptr);
    memoryPool.Deallocate(&struct3);
    assert(struct3 == nullptr);
    // Allocate again
    struct1 = memoryPool.Allocate();
    assert(struct1 != nullptr);
    struct3 = memoryPool.Allocate();
    assert(struct3 != nullptr);
    assert(memoryPool.Allocate() == nullptr);

    memoryPool.Cleanup();
}

struct MyKey
{
    uint64_t whatever;
};
template <>
struct std::hash<MyKey>
{
    std::size_t operator()(const MyKey& p) const noexcept
    {
        return std::hash<uint64_t>()(p.whatever);
    }
};
void TestCompHashMap()
{
    using namespace Chinstrap::Memory;

    CompHashMap<MyKey, TestStruct> hashMap;
    hashMap.Setup(2);

    const MyKey key = {1};
    auto myStruct = TestStruct();
    auto myStruct2 = TestStruct();
    myStruct2.number = 12.5f;
    MyKey key2 = {2};
    {
        auto ret = hashMap.Insert(key2, myStruct2);
        assert(ret == HashInsertRet::SUCCESS);
    }
    myStruct.number = 10.0f;
    {
        auto ret = hashMap.Insert(key, myStruct);
        assert(ret == HashInsertRet::SUCCESS);
    }

    hashMap.EndSetup();

    assert(hashMap.Lookup(key2).value()->number == myStruct2.number);
    assert(hashMap.Lookup(key).value()->number == myStruct.number);

    hashMap.Cleanup();
}
