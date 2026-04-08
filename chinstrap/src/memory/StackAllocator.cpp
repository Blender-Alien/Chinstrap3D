#include "StackAllocator.h"

#include "../ops/Logging.h"

using namespace Chinstrap::Memory;

void StackAllocator::Setup(const uint32_t stackSizeInBytes_arg)
{
    basePointer = static_cast<std::byte*>(malloc(stackSizeInBytes_arg));
    ENSURE_OR_RETURN_MSG((basePointer != nullptr), "StackAllocator failed to setup!");
    stackPointer = basePointer;
    topPointer = basePointer;
    stackSizeInBytes = stackSizeInBytes_arg;
}

void StackAllocator::Cleanup()
{
    free(basePointer);
    basePointer = nullptr;
    topPointer = nullptr;
    stackSizeInBytes = 0;
}

void StackAllocator::CleanupKeepData()
{
    basePointer = nullptr;
    topPointer = nullptr;
    stackSizeInBytes = 0;
}

StackAllocator::~StackAllocator()
{
    // Did we call Cleanup???
    assert(basePointer == nullptr);
}

std::byte* StackAllocator::DirectAllocate(const uint32_t sizeInBytes_arg, std::size_t alignment_arg)
{
    alignment_arg = std::max(static_cast<std::size_t>(4), alignment_arg);
    auto current = reinterpret_cast<std::uintptr_t>(topPointer);
    std::uintptr_t aligned = (current + alignment_arg - 1) & ~(alignment_arg - 1);
    std::size_t padding = aligned - current;

    if (topPointer + padding + sizeInBytes_arg > basePointer + stackSizeInBytes)
    {
        CHIN_LOG_ERROR("Stack allocator out of memory!");
        return nullptr;
    }
    topPointer += padding + sizeInBytes_arg;
    stackPointer = topPointer - sizeInBytes_arg;

    return stackPointer;
}

void StackAllocator::ClearStack()
{
    stackPointer = basePointer;
    topPointer = basePointer;
}