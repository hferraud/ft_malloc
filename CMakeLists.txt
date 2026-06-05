cmake_minimum_required(VERSION 3.22.1)

project(malloc)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(malloc SHARED)

target_sources(malloc PRIVATE
        ${SRC_DIR}/malloc.c
        ${SRC_DIR}/realloc.c
        ${SRC_DIR}/free.c
        ${SRC_DIR}/chunk.c
        ${SRC_DIR}/zone.c
        ${SRC_DIR}/display.c
)

target_include_directories(malloc PUBLIC
        ${INCLUDE_DIR}
)

target_compile_options(malloc PRIVATE
        -Wall
        -Werror
        -Wextra
)

option(BUILD_TEST "Build tests" OFF)

if(BUILD_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
else()
    add_executable(main
            ${SRC_DIR}/main.c
    )

    target_compile_options(main PRIVATE
            -Wall
            -Werror
            -Wextra
    )

    target_link_libraries(main malloc)
endif()
