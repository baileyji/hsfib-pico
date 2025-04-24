add_library(pico-ads1x15 INTERFACE)

target_include_directories(pico-ads1x15
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pico-ads1x15
        INTERFACE
        hardware_i2c
)

target_sources(pico-ads1x15
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ads1x15.cpp
        ${CMAKE_CURRENT_LIST_DIR}/include/ads1x15.h
)

