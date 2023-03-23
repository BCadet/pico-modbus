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
    gcc-arm-none-eabi-10.3-2021.10-x86_64-linux
    URL https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
    URL_MD5 2383e4eb4ea23f248d33adc70dc3227e
)

FetchContent_MakeAvailable(gcc-arm-none-eabi-10.3-2021.10-x86_64-linux)

FetchContent_Populate(pico-sdk)

set(PICO_SDK_PATH ${pico-sdk_SOURCE_DIR} CACHE FILEPATH "PICO_SDK_PATH")
set(PICO_TOOLCHAIN_PATH ${gcc-arm-none-eabi-10.3-2021.10-x86_64-linux_SOURCE_DIR} CACHE FILEPATH "PICO_TOOLCHAIN_PATH")

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

macro(panduza_platfrom_init TARGET)
    pico_sdk_init()
    # find gdb for debugger (pico_sdk_init forget to set it).
    # see build/_deps/pico-sdk-src/cmake/preload/toolchains/pico_arm_gcc.cmake)
    # find_program(
    #         PICO_COMPILER_GDB arm-none-eabi-gdb
    #         PATHS ENV PICO_TOOLCHAIN_PATH
    #         PATH_SUFFIXES bin
    #         NO_DEFAULT_PATH
    # )
    pico_find_compiler(PICO_COMPILER_GDB arm-none-eabi-gdb)
    set(CMAKE_DEBUGGER ${PICO_COMPILER_GDB} CACHE FILEPATH "path to gdb")

    target_compile_definitions(panduza_platform PRIVATE
        -DCFG_TUSB_CONFIG_FILE="pico_tusb_config.h"
        -DPICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=-1 # wait in stdio_usb_init until usb is connected
        # -DPICO_STDIO_USB_POST_CONNECT_WAIT_DELAY_MS=1000 # wait 1s AFTER the usb is conected
        -DPICO_STDIO_USB_LOW_PRIORITY_IRQ=0x1f
    )

    # enable usb output, disable uart output
    pico_enable_stdio_uart(${TARGET} 0)
    pico_enable_stdio_usb(${TARGET} 1)
    # create uf2 file
    pico_add_uf2_output(${TARGET})
    # aditionnal cleanup
    set_property(
        TARGET ${TARGET}
        APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES 
        ${TARGET}.bin
        ${TARGET}.dis
        ${TARGET}.elf.map
        ${TARGET}.hex
        ${TARGET}.uf2
    )

    # exclude stdio_usb_descriptors.c from compilation of pico_stdio_usb lib to use mine instead
    set_source_files_properties(
        ${pico-sdk_SOURCE_DIR}/src/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c
        PROPERTIES HEADER_FILE_ONLY ON
    )

endmacro()