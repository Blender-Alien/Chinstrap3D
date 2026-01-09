# core

set(SOURCES
    src/Application.cpp
    src/Application.h
)

add_library(core STATIC)
target_sources(core PRIVATE ${SOURCES})

target_link_libraries(core glfw)

target_include_directories(core INTERFACE "src") 

