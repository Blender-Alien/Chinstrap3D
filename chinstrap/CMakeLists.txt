# chinstrap

set(SOURCES
        src/Logging.h
        src/Application.cpp
        src/Application.h
        src/Window.cpp
        src/Window.h
        src/Scene.cpp
        src/Scene.h
        src/Event.h
        src/WindowEvents.h
        src/InputEvents.h
        src/Renderer.h
        src/Renderer.cpp
)

add_library(chinstrap STATIC)
target_sources(chinstrap PRIVATE ${SOURCES})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        GLEW_STATIC
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(chinstrap glfw)
target_link_libraries(chinstrap glad)
# Note that excluding spdlog from Release doesn't work as of 22.01.2026, https://gitlab.kitware.com/cmake/cmake/-/issues/24629
target_link_libraries(chinstrap $<$<CONFIG:DEBUG>:spdlog::spdlog>)

target_include_directories(chinstrap INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_precompile_headers(chinstrap
    PRIVATE pch.h
)
