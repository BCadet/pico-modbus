add_library(panduza_dio)
add_library(panduza::dio ALIAS panduza_dio)

target_sources(panduza_dio PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/panduza_dio.c
)

target_include_directories(panduza_dio PRIVATE
${CMAKE_CURRENT_LIST_DIR}/inc
)

target_include_directories(panduza_dio INTERFACE
${CMAKE_CURRENT_LIST_DIR}/inc
)