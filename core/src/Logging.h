#pragma once

#if CHIN_DEBUG
#include "spdlog/spdlog.h"

#define CHIN_INFO(x) spdlog::info(x)
#define CHIN_WARN(x) spdlog::warn(x)
#define CHIN_ERROR(x) spdlog::error(x)
#define CHIN_CRITICAL(x) spdlog::critical(x)

#elif CHIN_RELEASE

#define CHIN_INFO(x)
#define CHIN_WARN(x)
#define CHIN_ERROR(x)
#define CHIN_CRITICAL(x)

#endif
