cmake_minimum_required(VERSION 3.13)

set(BOARD_NAME W5500_EVB_PICO2)

#FetchContent_Declare( freertos_kernel
#        GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
#        GIT_TAG        main #Note: Best practice to use specific git-hash or tagged version
#)

# Set WIZchip Clock Speed
add_definitions(-D_WIZCHIP_SPI_SCLK_SPEED=40) # SPEED MHz
add_definitions(-DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

set(PICO_PLATFORM rp2350)
set(WIZNET_CHIP W5500)
add_definitions(-D_WIZCHIP_=W5500)
add_definitions(-DDEVICE_BOARD_NAME=W5500_EVB_PICO2)


if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/lib/pico-sdk)
    message(STATUS "PICO_SDK_PATH = ${PICO_SDK_PATH}")
endif()

include(pico_sdk_import.cmake)

# Set project name
set(PROJECT_NAME HISPEC-FIB-PICO2)
project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()


if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/lib/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

if(NOT DEFINED MBEDTLS_DIR)
    set(MBEDTLS_DIR ${CMAKE_SOURCE_DIR}/lib/pico-sdk/lib/mbedtls)
    message(STATUS "MBEDTLS_DIR = ${MBEDTLS_DIR}")
endif()

if(NOT DEFINED PORT_DIR)
    set(PORT_DIR ${CMAKE_SOURCE_DIR}/lib/WIZnet-PICO-C/port)
    message(STATUS "PORT_DIR = ${PORT_DIR}")
endif()

# Turn off mbedtls test mode
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbedtls programs")
set(ENABLE_TESTING OFF CACHE BOOL "Build mbedtls testing")
add_definitions(-DMBEDTLS_CONFIG_FILE="${PORT_DIR}/mbedtls/inc/ssl_config.h")
add_definitions(-DSET_TRUSTED_CERT_IN_SAMPLES)

# Hardware-specific examples in subdirectories:
add_subdirectory(programs)

# Add libraries in subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(${MBEDTLS_DIR})
add_subdirectory(${PORT_DIR})


# Set compile options
add_compile_options(
        -Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)