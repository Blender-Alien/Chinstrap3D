#include "StackAllocator.h"

#include "../ops/Logging.h"

using namespace Chinstrap::Memory;

void StackAllocator::Setup(const uint32_t stackSizeInBytes_arg)
{
    basePointer = static_cast<std::byte*>(malloc(stackSizeInBytes_arg));
    assert(basePointer != nullptr);
    this->stackPointer = basePointer;
    this->topPointer = basePointer;

    this->stackSizeInBytes = stackSizeInBytes_arg;
}

void StackAllocator::Cleanup()
{
    free(basePointer);
    basePointer = nullptr;
    topPointer = nullptr;
}

StackAllocator::~StackAllocator()
{
    // Did we call Cleanup???
    assert(basePointer == nullptr);
}

std::byte* StackAllocator::DirectAllocate(const uint32_t sizeInBytes_arg)
{
    if (topPointer + sizeInBytes_arg > basePointer + stackSizeInBytes)
    {
        CHIN_LOG_ERROR("Stack allocator out of memory!");
        return nullptr;
    }
    topPointer += sizeInBytes_arg;
    stackPointer = topPointer - sizeInBytes_arg;

    return stackPointer;
}

void StackAllocator::ClearStack()
{
    stackPointer = basePointer;
    topPointer = basePointer;
}
