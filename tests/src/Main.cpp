#include "chinstrap/src/ops/Logging.h"

#include "MemoryTest.h"

int main()
{
    CHIN_LOG_INFO("Testing...");

    TestStackArray();
    TestStackArray2D();

    TestMemoryPool();

    TestFilePathMap();

    CHIN_LOG_INFO("All tests passed!");

#ifndef CHIN_DEBUG
    std::cout << "WARNING: Tests will not work correctly, assertions are not enabled in release modes!" << std::endl;
#endif
}