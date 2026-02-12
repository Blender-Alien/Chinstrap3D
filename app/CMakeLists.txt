# app

file(GLOB sources
        "src/*.cpp"
        "src/*.h"

        "../vendor/fonts/Roboto/Roboto-Regular.h"
)

add_executable(app ${sources})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(app chinstrap)

target_include_directories(app PRIVATE src)

target_precompile_headers(app PRIVATE pch.h)
