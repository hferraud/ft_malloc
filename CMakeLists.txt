cmake_minimum_required(VERSION 3.22.1)

project(malloc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

if (DEFINED ENV{HOSTTYPE})
    set (HOST_TYPE "$ENV{HOSTTYPE}")
else ()
    set(HOST_TYPE "${CMAKE_HOST_SYSTEM_PROCESSOR}_${CMAKE_HOST_SYSTEM_NAME}")
endif ()

message(STATUS "HOSTTYPE=${HOST_TYPE}")

set(LIB_MALLOC "libft_malloc_${HOST_TYPE}.so")
set(LIB_LINK "libft_malloc.so")

add_custom_target(create_symlink ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${LIB_MALLOC}
        ${LIB_LINK}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_library(malloc SHARED
        src/main.c
)

target_sources(malloc PRIVATE
        ${SRC_DIR}/malloc.c
        ${SRC_DIR}/realloc.c
        ${SRC_DIR}/calloc.c
        ${SRC_DIR}/free.c
        ${SRC_DIR}/chunk.c
        ${SRC_DIR}/zone.c
        ${SRC_DIR}/display.c
        ${SRC_DIR}/utils.c
        ${SRC_DIR}/memory.c
)

target_include_directories(malloc PUBLIC
        ${INCLUDE_DIR}
)

target_compile_options(malloc PRIVATE
        -Wall
        -Werror
        -Wextra
)

set_target_properties(malloc PROPERTIES
        OUTPUT_NAME "ft_malloc_${HOST_TYPE}"
)

option(BUILD_TEST "Build tests" OFF)

if(BUILD_TEST)
    target_compile_options(malloc PUBLIC
            -g3
    )
    include(CTest)
    enable_testing()
    add_subdirectory(test)
else()
    add_executable(main
            ${SRC_DIR}/main.c
            include/utils.h
            src/utils.c
            src/calloc.c
            include/calloc.h
            include/memory.h
            src/memory.c
    )

    target_compile_options(main PRIVATE
            -Wall
            -Werror
            -Wextra
    )

    target_link_libraries(main malloc)
endif()
