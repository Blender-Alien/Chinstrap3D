# app

set(SOURCES
    src/Main.cpp
)

add_executable(app) 
target_sources(app PRIVATE ${SOURCES})


target_link_libraries(app core)

target_include_directories(app PRIVATE src)


