# chinstrap

file(GLOB sources
        "src/*.cpp"
        "src/*.h"
        "src/ops/*.cpp"
        "src/ops/*.h"
        "src/events/*.cpp"
        "src/events/*.h"
        "src/rendering/*.cpp"
        "src/rendering/*.h"
        "src/memory/*.cpp"
        "src/memory/*.h"

        "../vendor/fonts/Roboto/Roboto-Regular.h"
)

add_library(chinstrap STATIC ${sources})

add_compile_definitions(
        GLFW_INCLUDE_NONE
        $<$<CONFIG:Debug>:CHIN_DEBUG>
        $<$<CONFIG:Debug>:CHIN_VK_VAL_LAYERS>
        $<$<CONFIG:Release>:CHIN_RELEASE>
)

target_link_libraries(chinstrap Vulkan::Vulkan)
target_link_libraries(chinstrap glfw)
target_link_libraries(chinstrap spdlog::spdlog)
target_link_libraries(chinstrap imgui)
target_link_libraries(chinstrap glm)

target_include_directories(chinstrap INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. PRIVATE ../vendor/fonts/)

target_precompile_headers(chinstrap PRIVATE pch.h)
