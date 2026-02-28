# app

file(GLOB sources
        "src/*.cpp"
        "src/*.h"
)

add_executable(tests ${sources})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(tests chinstrap)

target_include_directories(tests PRIVATE src)
target_precompile_headers(tests PRIVATE ../chinstrap/pch.h)
