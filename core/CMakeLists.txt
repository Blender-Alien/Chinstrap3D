# core

set(SOURCES
        Chinstrap.h
        src/Application.cpp
        src/Application.h
        src/Window.cpp
        src/Window.h
        src/Scene.cpp
        src/Scene.h
        src/Event.h
        src/WindowEvents.h
)

add_library(core STATIC)
target_sources(core PRIVATE ${SOURCES})

add_compile_definitions(
    GLFW_INCLUDE_NONE
    GLEW_STATIC
)

target_link_libraries(core glfw)
target_link_libraries(core glad)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) 

