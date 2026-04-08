#include "MemoryTest.h"

int main()
{

    TestStackArray();
    TestStackArray2D();

    TestMemoryPool();

    TestCompHashMap();

#ifndef CHIN_DEBUG
    std::cout << "WARNING: Tests will not work correctly, assertions are not enabled in release modes!" << std::endl;
#endif
}