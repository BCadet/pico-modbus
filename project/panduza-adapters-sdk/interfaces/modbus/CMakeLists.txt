include(FetchContent)

FetchContent_Declare(
    liblightmodbus
    GIT_REPOSITORY https://github.com/Jacajack/liblightmodbus.git
    GIT_TAG v3.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES_RECURSE FALSE
)

FetchContent_MakeAvailable(liblightmodbus)

add_library(modbus)

target_sources(modbus PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/modbus.c
    ${CMAKE_CURRENT_LIST_DIR}/modbusDevice.c
)

target_include_directories(modbus PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${liblightmodbus_SOURCE_DIR}/include/
)

target_include_directories(modbus INTERFACE
    ${liblightmodbus_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

option(MODBUS_LOG "enable modbus access debug log" False)

if(${MODBUS_LOG})
    add_definitions("-DMODBUS_LOG")
endif()