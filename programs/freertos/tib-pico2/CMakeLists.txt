# =========================
# Project Target
# =========================
set(TARGET_NAME tib_pico2)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.cpp
        photodiode_task.cpp photodiode_task.h
        photodiode.cpp photodiode.h
        attenuator.cpp attenuator.h
        mems_switching.cpp mems_switching.h
        pico_zyre.cpp pico_zyre.h
        coms_task.cpp coms_task.h
        executor_task.cpp executor_task.h
        keys.h
        hardware_context.h
)

# =========================
# Include Paths
# =========================
target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/
)

# =========================
# Compile and Link Options
# =========================
target_compile_options(${TARGET_NAME} PRIVATE
)

target_link_options(${TARGET_NAME} PRIVATE
        -Wl,-Map=${TARGET_NAME}.map
        -Wl,--print-memory-usage
        -Wl,--cref
)

# =========================
# Definitions (compile-time settings)
# =========================
target_compile_definitions(${TARGET_NAME} PRIVATE
        _WIZCHIP_SPI_SCLK_SPEED=40
        configNUMBER_OF_CORES=2
)

# =========================
# Libraries
# =========================
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
        nlohmann_json::nlohmann_json
)

# =========================
# StdIO (USB/UART)
# =========================
pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

# =========================
# Extra Outputs (bin, hex, uf2)
# =========================
pico_add_extra_outputs(${TARGET_NAME})

# =========================
# Custom Post-Build Steps
# =========================
find_program(CMAKE_SIZE arm-none-eabi-size)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TARGET_NAME}>
        COMMENT "Generate binary and show memory usage"
)