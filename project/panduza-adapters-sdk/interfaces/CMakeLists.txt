if(NOT PANDUZA_INTERFACE)
message(FATAL_ERROR "Please define a PANDUZA_INTERFACE")
endif()

add_subdirectory(${PANDUZA_INTERFACE})

add_library(panduza::interface ALIAS ${PANDUZA_INTERFACE})
