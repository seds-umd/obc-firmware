project("OBC Tests" C)
cmake_minimum_required(VERSION 3.0)

# Set up unity library
add_library(unity STATIC
    ../external/Unity/src/unity.c
)

target_include_directories(unity PUBLIC
    ../external/Unity/src
)

# Add source headers
include_directories(
    ../include
)

add_executable(tests
    # Add new sources files here
    ../src/commands.c

    # Don't add unit test files here, only add them at the top of test/main.c
    main.c

    # Don't add src/main.c because it will conflict with test/main.c
)

# Add library
target_link_libraries(tests unity)

# Turn on basically all useful warnings
# (if this is a problem we can turn some off later)
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -fanalyzer
)
