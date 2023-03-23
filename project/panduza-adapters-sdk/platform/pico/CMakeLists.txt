add_library(panduza_platform_pico)

target_sources(panduza_platform_pico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/modbus.c
    ${CMAKE_CURRENT_LIST_DIR}/dio.c
    ${CMAKE_CURRENT_LIST_DIR}/platform.c
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/uart.c
    ${CMAKE_CURRENT_LIST_DIR}/spi.c
)

target_include_directories(panduza_platform_pico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(panduza_platform_pico
    pico_stdlib
    pico_stdio_usb
    pico_bootrom
    hardware_uart
    hardware_spi
    modbus
)
