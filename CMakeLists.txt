cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER   "clang"   CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)

project(Chinstrap3D)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(Dependencies.cmake)

set(CHIN_SHIPPING_BUILD OFF CACHE BOOL "Disable human readable serialization, resizable data structures etc...")

add_subdirectory(chinstrap)
add_subdirectory(tests)
add_subdirectory(app)

set_property(TARGET app PROPERTY CXX_STANDARD 17)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)
set_property(TARGET chinstrap PROPERTY CXX_STANDARD 17)
