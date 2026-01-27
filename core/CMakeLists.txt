# core

set(SOURCES
        Chinstrap.h
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

add_library(core STATIC)
target_sources(core PRIVATE ${SOURCES})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        GLEW_STATIC
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(core glfw)
target_link_libraries(core glad)
# Note that excluding spdlog from Release doesn't work as of 22.01.2026, https://gitlab.kitware.com/cmake/cmake/-/issues/24629
target_link_libraries(core $<$<CONFIG:DEBUG>:spdlog::spdlog>)

target_include_directories(core PUBLIC
        /
)

