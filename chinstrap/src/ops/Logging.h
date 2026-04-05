#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

inline std::shared_ptr<spdlog::logger> fileLogger;

#ifndef CHIN_SHIPPING_BUILD

#define CHIN_LOG_INFO(...)     { fileLogger.get()->info(__VA_ARGS__);     spdlog::info(__VA_ARGS__); }
#define CHIN_LOG_WARN(...)     { fileLogger.get()->warn(__VA_ARGS__);     spdlog::warn(__VA_ARGS__); }
#define CHIN_LOG_ERROR(...)    { fileLogger.get()->error(__VA_ARGS__);    spdlog::error(__VA_ARGS__); }
#define CHIN_LOG_CRITICAL(...) { fileLogger.get()->critical(__VA_ARGS__); spdlog::critical(__VA_ARGS__); }

#define CHIN_LOG_INFO_VULKAN_F(x, ...) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_INFO_VULKAN(x) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_WARN_VULKAN_F(x, ...) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_WARN_VULKAN(x) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_ERROR_VULKAN_F(x, ...) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_ERROR_VULKAN(x) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_CRITICAL_VULKAN_F(x, ...) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_CRITICAL_VULKAN(x) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x))

#else // In a shipping build we still want logging but not only into a logFile

#define CHIN_LOG_INFO(...)     fileLogger.get()->info(__VA_ARGS__)
#define CHIN_LOG_WARN(...)     fileLogger.get()->warn(__VA_ARGS__)
#define CHIN_LOG_ERROR(...)    fileLogger.get()->error(__VA_ARGS__)
#define CHIN_LOG_CRITICAL(...) fileLogger.get()->critical(__VA_ARGS__)

#define CHIN_LOG_INFO_VULKAN_F(x, ...) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_INFO_VULKAN(x) CHIN_LOG_INFO(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_WARN_VULKAN_F(x, ...) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_WARN_VULKAN(x) CHIN_LOG_WARN(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_ERROR_VULKAN_F(x, ...) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_ERROR_VULKAN(x) CHIN_LOG_ERROR(std::string("[Vulkan] ") + std::string(x))

#define CHIN_LOG_CRITICAL_VULKAN_F(x, ...) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x), __VA_ARGS__)
#define CHIN_LOG_CRITICAL_VULKAN(x) CHIN_LOG_CRITICAL(std::string("[Vulkan] ") + std::string(x))

#endif