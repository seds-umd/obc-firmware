project("OBC Tests" C)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_BUILD_TYPE Debug)

# Set up unity library
add_library(unity STATIC
    ../external/Unity/src/unity.c
)

target_include_directories(unity PUBLIC
    ../external/Unity/src
)

include_directories(
    .
    ../include # Project source headers
)

add_executable(tests
    # Add new sources files here
    ../src/commands.c
    ../src/scheduler.c

    main.c
    globals.c
    # Add files containing tests here:
    test_commands.c
    test_scheduler.c

    # Don't add src/main.c because it will conflict with test/main.c
)

# Add library
target_link_libraries(tests unity)

# Turn on basically all useful warnings
# (if this is a problem we can turn some off later)
target_compile_options(tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -ggdb3
    # -fanalyzer # Not available on GCC 9
)

target_compile_definitions(tests PRIVATE
    SCHEDULER_MAX_TASKS=3
)
