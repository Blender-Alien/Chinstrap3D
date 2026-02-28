#include "chinstrap/src/ops/Logging.h"

#include "MemoryTest.h"

int main()
{
    CHIN_LOG_INFO("Testing...");

    TestStackArray();
    TestStackArray2D();

    TestMemoryPool();

    CHIN_LOG_INFO("All tests passed!");
}