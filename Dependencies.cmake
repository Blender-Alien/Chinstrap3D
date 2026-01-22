# OpenGL
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw-3.4)

# glad
set(SOURCES
    vendor/glad/src/glad.c
    vendor/glad/src/glad.h
)

add_library(glad STATIC)
target_sources(glad PRIVATE ${SOURCES})

target_include_directories(glad PUBLIC "vendor/glad/src") 

# spdlog
find_package(spdlog REQUIRED)
