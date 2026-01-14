cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER   "clang"   CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)

project(Chinstrap3D)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(Dependencies.cmake)


add_subdirectory(core)
add_subdirectory(app)


