cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(obc_fw C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Internal and external includes
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/littlefs
)

add_executable(obc_fw
    # Internal source files
    src/commands.c
    src/command_handler.c
    src/main.c
    src/openlst.c
    src/scheduler.c

    # External source files
    external/littlefs/lfs.c
)

pico_enable_stdio_usb(obc_fw 1)

target_link_libraries(obc_fw
    pico_multicore
    pico_stdlib
    pico_rand
    hardware_dma
    hardware_spi
)

add_compile_definitions(
    LFS_NO_MALLOC # Disable dynamic memory allocation in LFS
)

# Turn on basically all useful warnings
# (if this is a problem we can turn some off later)
target_compile_options(obc_fw PUBLIC
    -Wall
    -Wextra
    -fstack-usage
)

pico_add_extra_outputs(obc_fw)

install(CODE "execute_process(
    COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c \"adapter speed 5000; program ${CMAKE_PROJECT_NAME}.elf verify reset exit\"
)")
