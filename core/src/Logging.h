#pragma once

#if CHIN_DEBUG
#include "spdlog/spdlog.h"

#define CHIN_LOG_INFO(x) spdlog::info(x)
#define CHIN_LOG_WARN(x) spdlog::warn(x)
#define CHIN_LOG_ERROR(x) spdlog::error(x)
#define CHIN_LOG_CRITICAL(x) spdlog::critical(x)

#elif CHIN_RELEASE

#define CHIN_LOG_INFO(x)
#define CHIN_LOG_WARN(x)
#define CHIN_LOG_ERROR(x)
#define CHIN_LOG_CRITICAL(x)

#endif
