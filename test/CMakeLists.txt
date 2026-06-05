include(FetchContent)

FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.1
)

FetchContent_MakeAvailable(unity)

file(GLOB TEST_FILES
        test_*.c
)

foreach(TEST_FILE ${TEST_FILES})

    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    add_executable(${TEST_NAME}
            ${TEST_FILE}
    )

    target_link_libraries(${TEST_NAME}
            PRIVATE
            malloc
            unity
    )

    add_test(
            NAME ${TEST_NAME}
            COMMAND ${TEST_NAME}
    )
endforeach()