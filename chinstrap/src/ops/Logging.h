#pragma once
#include "spdlog/spdlog.h"

#if CHIN_DEBUG

#define CHIN_LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define CHIN_LOG_WARN(...) spdlog::warn(__VA_ARGS__)

#define CHIN_LOG_INFO_VULKAN_F(x, ...) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_INFO_VULKAN(x) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_WARN_VULKAN_F(x, ...) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_WARN_VULKAN(x) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x))

#elif CHIN_RELEASE

#define CHIN_LOG_INFO(...)
#define CHIN_LOG_WARN(...)

#define CHIN_LOG_INFO_VULKAN_F(x, ...)
#define CHIN_LOG_INFO_VULKAN(x)
#define CHIN_LOG_WARN_VULKAN_F(x, ...)
#define CHIN_LOG_WARN_VULKAN(x, ...)

#endif

#define CHIN_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define CHIN_LOG_CRITICAL(...) spdlog::critical(__VA_ARGS__)

#define CHIN_LOG_ERROR_VULKAN_F(x, ...) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_ERROR_VULKAN(x) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_CRITICAL_VULKAN_F(x, ...) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_CRITICAL_VULKAN(x) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x))
