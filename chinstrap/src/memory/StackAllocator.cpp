#include "StackAllocator.h"

using namespace Chinstrap::Memory;

StackAllocator::StackAllocator(const uint32_t stackSizeInBytes_arg)
{
    this->basePointer = static_cast<uint32_t *>(malloc(stackSizeInBytes_arg));

    this->stackPointer = basePointer;
    this->topPointer = basePointer;

    this->stackSizeInBytes = stackSizeInBytes_arg;
}

StackAllocator::~StackAllocator()
{
    free(this->basePointer);
}

void StackAllocator::ClearStack()
{
    stackPointer = basePointer;
    topPointer = basePointer;
}
