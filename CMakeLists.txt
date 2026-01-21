cmake_minimum_required(VERSION 3.22.1)
project(malloc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(malloc SHARED)

target_sources(malloc PRIVATE
        ${SRC_DIR}/malloc.c
)

target_include_directories(malloc PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(malloc PRIVATE
        -Wall
        -Werror
        -Wextra
)

add_executable(main
        ${SRC_DIR}/main.c
)

target_compile_options(main PRIVATE
        -Wall
        -Werror
        -Wextra
)

target_link_libraries(main malloc)
