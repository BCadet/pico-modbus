cmake_minimum_required(VERSION 3.16)
# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # set cmake policy regarding the handling of hard defined options
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    pico-sdk
    GIT_REPOSITORY  https://github.com/raspberrypi/pico-sdk.git
    GIT_TAG 1.5.0
)

FetchContent_Declare(
    liblightmodbus
    GIT_REPOSITORY https://github.com/Jacajack/liblightmodbus.git
    GIT_TAG v3.0
    GIT_SUBMODULES ""
)

FetchContent_Declare(
    gcc-arm-none-eabi-10.3-2021.10-x86_64-linux
    URL https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
)

FetchContent_Declare(
    RV-3028-C7
    GIT_REPOSITORY https://github.com/Kampi/RV-3028-C7.git
    GIT_TAG 1.0
)


FetchContent_MakeAvailable(liblightmodbus)
FetchContent_MakeAvailable(gcc-arm-none-eabi-10.3-2021.10-x86_64-linux)
FetchContent_MakeAvailable(RV-3028-C7)

set(PICO_SDK_PATH ${pico-sdk_SOURCE_DIR})
set(PICO_TOOLCHAIN_PATH ${gcc-arm-none-eabi-10.3-2021.10-x86_64-linux_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")

# add_library(_pico_sdk_inclusion_marker INTERFACE) # avoid execution of the CMakeLists.txtx of the pico-sdk at MakeAvailable operation
FetchContent_Populate(pico-sdk)
include(${pico-sdk_SOURCE_DIR}/pico_sdk_init.cmake)
pico_sdk_init()

project(pico-modbus)
# set(CMAKE_DEBUGGER ${gcc-arm-none-eabi-10.3-2021.10-x86_64-linux_SOURCE_DIR}/bin/arm-none-eabi-gdb CACHE INTERNAL "debugger")

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/platform.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
    ${rv-3028-c7_SOURCE_DIR}/external/RV3028/RV3028.c
)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${liblightmodbus_SOURCE_DIR}/include/lightmodbus
    ${CMAKE_CURRENT_LIST_DIR}/inc/
    ${rv-3028-c7_SOURCE_DIR}/external/RV3028
)


target_link_libraries(${CMAKE_PROJECT_NAME}
    pico_stdlib
    tinyusb_device_unmarked
    hardware_i2c
)

add_definitions(
    -DCFG_TUSB_CONFIG_FILE="pico_tusb_config.h"
    -DLIB_TINYUSB_DEVICE=1
    -DPICO_I2C_SCL_PIN=15
    -DPICO_I2C_SDA_PIN=14
    )

# enable usb output, disable uart output
# pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
