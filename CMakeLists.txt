cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(obc_fw C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FLOW_CONTROL "OpenLST Flow Control" ON)

if(!FLOW_CONTROL)
    add_compile_definitions(-DNO_FLOW_CONTROL)
endif(!FLOW_CONTROL)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Internal and external includes
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/littlefs
)

# Program both application and bootloader
install(CODE "execute_process(
    COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c \"adapter speed 5000; program bootloader.elf; program application.elf verify reset exit\"
)")

# Reset RP2040 without programming
add_custom_target(reset
    COMMAND openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c \"init\; reset halt\; rp2040.core1 arp_reset assert 0\; rp2040.core0 arp_reset assert 0\; exit\"
    COMMENT "Resetting..."
)

###### Application ######

add_executable(application
    # Internal source files
    src/commands.c
    src/command_handler.c
    src/crc32.c
    src/flash.c
    src/logging.c
    src/main.c
    src/openlst.c
    src/openlst_driver.c
    src/scheduler.c
    src/telemetry.c
    src/updater.c

    # External source files
    external/littlefs/lfs.c
)

pico_enable_stdio_usb(application 1)

target_link_libraries(application
    pico_multicore
    pico_stdlib
    pico_rand
    hardware_dma
    hardware_spi
)

pico_set_linker_script(application ${CMAKE_SOURCE_DIR}/include/application.ld)

add_compile_definitions(
    LFS_NO_MALLOC # Disable dynamic memory allocation in LFS
)

# Turn on basically all useful warnings
# (if this is a problem we can turn some off later)
target_compile_options(application PUBLIC
    -Wall
    -Wextra
    -fstack-usage
)

pico_add_extra_outputs(application)

###### Bootloader ######
add_executable(bootloader
    src/bootloader/main.c
)

target_link_libraries(bootloader
    pico_stdlib
    hardware_flash
    cmsis_core
)

pico_set_linker_script(bootloader ${CMAKE_SOURCE_DIR}/include/bootloader.ld)

target_compile_options(bootloader PUBLIC
    -Wall
    -Wextra
    -fstack-usage
)

pico_add_extra_outputs(bootloader)
pico_set_binary_type(bootloader copy_to_ram)
