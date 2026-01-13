# core

set(SOURCES
    src/Application.cpp
    src/Application.h
)

add_library(core STATIC)
target_sources(core PRIVATE ${SOURCES})

add_compile_definitions(
    GLFW_INCLUDE_NONE
    GLEW_STATIC
)

target_link_libraries(core glfw)
target_link_libraries(core glad)

target_include_directories(core INTERFACE "src") 

