# chinstrap

set(SOURCES
        src/ops/Logging.h
        src/Application.cpp
        src/Application.h
        src/Window.cpp
        src/Window.h
        src/Scene.cpp
        src/Scene.h
        src/events/Event.h
        src/events/WindowEvents.h
        src/events/InputEvents.h
        src/rendering/Renderer.h
        src/rendering/Renderer.cpp
        src/ops/DevInterface.h
        src/ops/DevInterface.cpp
        src/ops/Profiling.h

        ../vendor/fonts/Roboto/Roboto-Regular.h
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
target_link_libraries(chinstrap imgui)

target_include_directories(chinstrap INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. PRIVATE ../vendor/fonts/)

target_precompile_headers(chinstrap
    PRIVATE pch.h
)
