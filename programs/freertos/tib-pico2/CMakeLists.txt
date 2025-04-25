set(TARGET_NAME tib_pico2)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.cpp
        photodiode_task.cpp
        photodiode_task.h
        photodiode.cpp
        photodiode.h
        attenuator.cpp
        attenuator.h
        attenuator_task.cpp
        attenuator_task.h
        mems_switching.cpp
        mems_switching.h
        switching_task.cpp
        switching_task.h
        pico_zyre.cpp
        pico_zyre.h
        coms_task.cpp
        coms_task.h
)

add_link_options(
        -Wl,-Map=${TARGET_NAME}.map
        -Wl,--print-memory-usage
        -Wl,--cref
)

find_program(CMAKE_SIZE arm-none-eabi-size)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
        COMMENT "Generate binary and show memory usage"
)

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/
)



target_link_libraries(${TARGET_NAME} PRIVATE
        pico_async_context_freertos
        pico-ads1x15
        pico-pcal6416a
        pico-dacx578
        FreeRTOS-Kernel-Heap4
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
        configNUMBER_OF_CORES=2
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})


