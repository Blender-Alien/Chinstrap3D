# app

set(SOURCES
        pch.h

        src/Main.cpp
        src/TestGLScene.cpp
        src/TestGLScene.h
        src/TestMenuScene.cpp
        src/TestMenuScene.h
        src/TestGuiScene.cpp
        src/TestGuiScene.h
)

add_executable(app) 
target_sources(app PRIVATE ${SOURCES})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(app chinstrap)
target_link_libraries(app imgui)

target_include_directories(app PRIVATE src)

target_precompile_headers(app PRIVATE pch.h)
