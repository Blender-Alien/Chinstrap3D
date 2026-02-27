# Vulkan
find_package(Vulkan REQUIRED)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw-3.4)

# spdlog
add_subdirectory("vendor/spdlog-1.17.0")

# imgui
set(IMGUI_PATH vendor/imgui-1.92.5)
file(GLOB IMGUI_GLOB
        ${IMGUI_PATH}/imgui.h
        ${IMGUI_PATH}/imgui.cpp
        ${IMGUI_PATH}/imconfig.h
        ${IMGUI_PATH}/imgui_demo.cpp
        ${IMGUI_PATH}/imgui_draw.cpp
        ${IMGUI_PATH}/imgui_internal.h
        ${IMGUI_PATH}/imstb_rectpack.h
        ${IMGUI_PATH}/imstb_textedit.h
        ${IMGUI_PATH}/imstb_truetype.h
        ${IMGUI_PATH}/imgui_tables.cpp
        ${IMGUI_PATH}/imgui_widgets.cpp

        ${IMGUI_PATH}/backends/imgui_impl_glfw.h
        ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.h
        ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
)
add_library(imgui STATIC ${IMGUI_GLOB})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)

# glm
add_subdirectory(vendor/glm-1.0.3)

# VMA

set(VMA_PATH vendor/VMA-3.3.0)
file(GLOB VMA_GLOB
        ${VMA_PATH}/include/vk_mem_alloc.h
        ${VMA_PATH}/src/VmaUsage.h
        ${VMA_PATH}/src/VmaUsage.cpp
)
add_library(VMA STATIC ${VMA_GLOB})
target_include_directories(VMA PUBLIC ${VMA_PATH}/include)
target_link_libraries(VMA PRIVATE Vulkan::Vulkan)

# json
set(JSON_PATH vendor/nlo-json-3.12.0)
file(GLOB JSON_GLOB
        ${JSON_PATH}/single_include/nlohmann/json.hpp
)
add_library(nlo-json STATIC ${JSON_GLOB})
set_target_properties(nlo-json PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(nlo-json PUBLIC ${JSON_PATH})