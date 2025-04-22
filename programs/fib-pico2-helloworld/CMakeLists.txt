set(TARGET_NAME fib_pico2_helloworld)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        ETHERNET_FILES
        IOLIBRARY_FILES
        DHCP_FILES
        DNS_FILES
        TIMER_FILES
)

target_compile_definitions(${TARGET_NAME} PRIVATE
        _WIZCHIP_SPI_SCLK_SPEED=40
)


pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})