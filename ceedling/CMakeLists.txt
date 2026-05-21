cmake_minimum_required(VERSION 3.22.1)
project(ceedling)

add_executable(ceedling EXCLUDE_FROM_ALL
        ceedling/test/test_chunk_create.c
        ceedling/test/test_zone_create.c
        ceedling/test/test_chunk_split.c
)

target_include_directories(ceedling PRIVATE
        ceedling/build/vendor/unity/src
)

target_link_libraries(ceedling PRIVATE
        malloc
)