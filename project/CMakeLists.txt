cmake_minimum_required(VERSION 3.17)
# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # set cmake policy regarding the handling of hard defined options
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(PANDUZA_PLATFORM "pico")
set(PANDUZA_INTERFACE "modbus")

include(FetchContent)
FetchContent_Declare(
    panduza-adapters-sdk
    GIT_REPOSITORY https://github.com/Panduza/panduza-adapters-sdk.git
    GIT_TAG add-i2c
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_Populate(panduza-adapters-sdk)
set(PANDUZA_SDK_PATH ${panduza-adapters-sdk_SOURCE_DIR})
include(${PANDUZA_SDK_PATH}/panduza_sdk_init.cmake)

project(pico-modbus)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

panduza_sdk_init(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc/
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    panduza::platform
    panduza::interface
    panduza::i2c
)