set(TARGET_NAME tib_pico2)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.cpp
        adc_task.cpp
        adc_task.h
        Photodiode.cpp
        Photodiode.h
        VariableAttenuator.cpp
        VariableAttenuator.h
        dac_task.cpp
        dac_task.h
        rep_task.cpp
        rep_task.h
        zmq_frame.cpp
        zmq_frame.h
        pub_task.cpp
        pub_task.h
        mems_switching.cpp
        mems_switching.h
        switching_task.cpp
        switching_task.h
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
