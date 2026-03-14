#include "MemoryTest.h"

#include "chinstrap/src/memory/StackArray.h"
#include "chinstrap/src/memory/StackAllocator.h"
#include "chinstrap/src/memory/MemoryPool.h"
#include "chinstrap/src/memory/FilePathMap.h"

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

void TestFilePathMap()
{
    using namespace Chinstrap::Memory;

    FilePathMap map;
    map.Setup(3, 24);

    FilePath path1;
    const char path1string[] = "/vendor/texture.png";

    FilePath path2;
    const char path2string[] = "/vendor/picture.png";

    FilePath path3;
    const char path3string[] = "/vendor/object.obj";

    {
        auto ret = map.Insert(path1, path1string);
        assert(ret == FilePathMap::InsertRet::SUCCESS);
    }
    {
        auto ret = map.Insert(path2, path2string);
        assert(ret == FilePathMap::InsertRet::SUCCESS);
    }
    {
        auto ret = map.Insert(path3, path3string);
        assert(ret == FilePathMap::InsertRet::SUCCESS);
    }

    map.EndSetup();

    {
        auto lookup1 = map.Lookup(path1);
        assert(lookup1.has_value());
        assert(lookup1 == path1string);
    }
    {
        auto lookup2 = map.Lookup(path2);
        assert(lookup2.has_value());
        assert(lookup2 == path2string);
    }
    {
        auto lookup3 = map.Lookup(path3);
        assert(lookup3.has_value());
        assert(lookup3 == path3string);
    }
    {
        FilePath path4;
        path4.hashID = std::hash<std::string_view>()("Hello there!");
        auto lookup4 = map.Lookup(path4);
        assert(!lookup4.has_value());
    }

    assert(!map.GrowTo(2, 24));
    assert(map.GrowTo(4, 24));

    {
        FilePath path5;
        path5.hashID = std::hash<std::string_view>()("Hello whats up?");
        auto ret = map.Insert(path5, "Hello whats up?");
        assert(ret == FilePathMap::InsertRet::SUCCESS);
        auto lookup5 = map.Lookup(path5);
        assert(lookup5.has_value());
        assert(lookup5 == "Hello whats up?");
    }

    map.Cleanup();
}
