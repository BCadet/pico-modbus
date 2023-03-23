add_library(panduza_platform)
add_library(panduza::platform ALIAS panduza_platform)

target_sources(panduza_platform PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/${PANDUZA_PLATFORM}/modbus.c
    ${CMAKE_CURRENT_LIST_DIR}/${PANDUZA_PLATFORM}/dio.c
    ${CMAKE_CURRENT_LIST_DIR}/${PANDUZA_PLATFORM}/platform.c
)

target_include_directories(panduza_platform PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/${PANDUZA_PLATFORM}/inc
)

target_include_directories(panduza_platform INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/${PANDUZA_PLATFORM}/inc
)

target_link_libraries(panduza_platform
    pico_stdlib
    pico_stdio_usb
    pico_bootrom
    modbus
)
