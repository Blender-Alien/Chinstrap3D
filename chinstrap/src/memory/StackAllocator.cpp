#include "StackAllocator.h"

#include "../ops/Logging.h"

using namespace Chinstrap::Memory;

void StackAllocator::Setup(const uint32_t stackSizeInBytes_arg)
{
    basePointer = static_cast<std::byte*>(malloc(sizeof(HeapStoredData) + stackSizeInBytes_arg));
    ENSURE_OR_RETURN_MSG((basePointer != nullptr), "StackAllocator failed to setup!");
    GetHeapData()->hStackPointer = basePointer + sizeof(HeapStoredData);
    GetHeapData()->hTopPointer = basePointer + sizeof(HeapStoredData);
    GetHeapData()->hStackSizeInBytes = stackSizeInBytes_arg;
}

void StackAllocator::Cleanup()
{
    free(basePointer);
    basePointer = nullptr;
}

void StackAllocator::CleanupKeepData()
{
    basePointer = nullptr;
}

StackAllocator::~StackAllocator()
{
    // Did we call Cleanup???
    assert(basePointer == nullptr);
}

std::byte* StackAllocator::DirectAllocate(const uint32_t sizeInBytes_arg, std::size_t alignment_arg)
{
    alignment_arg = std::max(static_cast<std::size_t>(4), alignment_arg);
    auto current = reinterpret_cast<std::uintptr_t>(GetHeapData()->hTopPointer);
    std::uintptr_t aligned = (current + alignment_arg - 1) & ~(alignment_arg - 1);
    std::size_t padding = aligned - current;

    if (GetHeapData()->hTopPointer + padding + sizeInBytes_arg > basePointer + GetHeapData()->hStackSizeInBytes + sizeof(HeapStoredData))
    {
        CHIN_LOG_ERROR("Stack allocator out of memory!");
        return nullptr;
    }
    GetHeapData()->hTopPointer += padding + sizeInBytes_arg;
    GetHeapData()->hStackPointer = GetHeapData()->hTopPointer - sizeInBytes_arg;

    return GetHeapData()->hStackPointer;
}

void StackAllocator::ClearStack()
{
    GetHeapData()->hStackPointer = basePointer + sizeof(HeapStoredData);
    GetHeapData()->hTopPointer = basePointer + sizeof(HeapStoredData);
}