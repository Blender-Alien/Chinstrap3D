#pragma once
#include "spdlog/spdlog.h"

#if CHIN_DEBUG

#define CHIN_LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define CHIN_LOG_WARN(...) spdlog::warn(__VA_ARGS__)

#elif CHIN_RELEASE

#define CHIN_LOG_INFO(...)
#define CHIN_LOG_WARN(...)

#endif

#define CHIN_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define CHIN_LOG_CRITICAL(...) spdlog::critical(__VA_ARGS__)
