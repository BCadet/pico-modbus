cmake_minimum_required(VERSION 3.17)
# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # set cmake policy regarding the handling of hard defined options
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    pico-sdk
    GIT_REPOSITORY  https://github.com/raspberrypi/pico-sdk.git
    GIT_TAG 1.5.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES_RECURSE FALSE
)

FetchContent_Declare(
    liblightmodbus
    GIT_REPOSITORY https://github.com/Jacajack/liblightmodbus.git
    GIT_TAG v3.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES_RECURSE FALSE
)

FetchContent_Declare(
    gcc-arm-none-eabi-10.3-2021.10-x86_64-linux
    URL https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
    URL_MD5 2383e4eb4ea23f248d33adc70dc3227e
)

FetchContent_MakeAvailable(liblightmodbus)
FetchContent_MakeAvailable(gcc-arm-none-eabi-10.3-2021.10-x86_64-linux)

set(PICO_SDK_PATH ${pico-sdk_SOURCE_DIR})
set(PICO_TOOLCHAIN_PATH ${gcc-arm-none-eabi-10.3-2021.10-x86_64-linux_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Release")

FetchContent_Populate(pico-sdk)
include(${pico-sdk_SOURCE_DIR}/pico_sdk_init.cmake)
project(pico-modbus)

pico_sdk_init()
# find gdb for debugger (pico_sdk_init forget to set it).
# see build/_deps/pico-sdk-src/cmake/preload/toolchains/pico_arm_gcc.cmake)
pico_find_compiler(PICO_COMPILER_GDB arm-none-eabi-gdb)
set(CMAKE_DEBUGGER ${PICO_COMPILER_GDB} CACHE FILEPATH "path to gdb")


set(PROJECT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/modbus.c
    ${CMAKE_CURRENT_LIST_DIR}/src/platform.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${liblightmodbus_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/inc/
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    pico_stdlib
    pico_stdio_usb
    hardware_i2c
    hardware_pwm
    pico_bootrom
)

add_definitions(
    -DCFG_TUSB_CONFIG_FILE="pico_tusb_config.h"
    -DPICO_I2C_SCL_PIN=15
    -DPICO_I2C_SDA_PIN=14
    -DPICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=-1 # wait in stdio_usb_init until usb is connected
    # -DPICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS=1000 # wait 1s AFTER the usb is conected
    )

# enable usb output, disable uart output
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
# create uf2 file
pico_add_uf2_output(${CMAKE_PROJECT_NAME})
# aditionnal cleanup
set_property(
    TARGET ${CMAKE_PROJECT_NAME}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.uf2
)
set_property(
    TARGET ${CMAKE_PROJECT_NAME}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map
)

# exclude stdio_usb_descriptors.c from compilation of pico_stdio_usb lib to use mine instead
set_source_files_properties(
    ${pico-sdk_SOURCE_DIR}/src/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c
    PROPERTIES HEADER_FILE_ONLY ON
)
