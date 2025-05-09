add_library(w5500-evb-pico2 INTERFACE)

target_sources(w5500-evb-pico2  INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/socket.c
        ${CMAKE_CURRENT_LIST_DIR}/wizchip_conf.c
        ${CMAKE_CURRENT_LIST_DIR}/wizchip_spi.c
        ${CMAKE_CURRENT_LIST_DIR}/wizchip_gpio_irq.c
        ${CMAKE_CURRENT_LIST_DIR}/w5500.c
        ${CMAKE_CURRENT_LIST_DIR}/timer.c
        ${CMAKE_CURRENT_LIST_DIR}/include/socket.h
        ${CMAKE_CURRENT_LIST_DIR}/include/wizchip_conf.h
        ${CMAKE_CURRENT_LIST_DIR}/include/wizchip_spi.h
        ${CMAKE_CURRENT_LIST_DIR}/include/wizchip_gpio_irq.h
        ${CMAKE_CURRENT_LIST_DIR}/include/W5500/w5500.h
        ${CMAKE_CURRENT_LIST_DIR}/include/timer.h
)

target_include_directories(w5500-evb-pico2 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(w5500-evb-pico2 INTERFACE
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
)

# DHCP
add_library(wiznet-dhcp INTERFACE)

target_sources(wiznet-dhcp INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/internet/DHCP/dhcp.c
)

target_include_directories(wiznet-dhcp INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/internet/DHCP
)


# DNS
add_library(wiznet-dns INTERFACE)

target_sources(wiznet-dns INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/internet/DNS/dns.c
)

target_include_directories(wiznet-dns INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/internet/DNS
)


#
## Loopback
#add_library(LOOPBACK_FILES STATIC)
#
#target_sources(LOOPBACK_FILES PUBLIC
#        ${WIZNET_DIR}/Application/loopback/loopback.c
#)
#
#target_include_directories(LOOPBACK_FILES PUBLIC
#        ${WIZNET_DIR}/Ethernet
#        ${WIZNET_DIR}/Application/loopback
#)
#
#
## AAC
#add_library(AAC_FILES STATIC)
#
#target_sources(AAC_FILES PUBLIC
#        ${WIZNET_DIR}/Internet/AAC/AddressAutoConfig.c
#)
#
#target_include_directories(AAC_FILES PUBLIC
#        ${WIZNET_DIR}/Ethernet
#        ${WIZNET_DIR}/Internet/AAC
#)



#
## SNTP
#add_library(SNTP_FILES STATIC)
#
#target_sources(SNTP_FILES PUBLIC
#        ${WIZNET_DIR}/Internet/SNTP/sntp.c
#)
#
#target_include_directories(SNTP_FILES PUBLIC
#        ${WIZNET_DIR}/Ethernet
#        ${WIZNET_DIR}/Internet/SNTP
#)
#
#
## multicast
#add_library(MULTI_FILES STATIC)
#
#target_sources(MULTI_FILES PUBLIC
#        ${WIZNET_DIR}/Application/multicast/multicast.c
#)
#
#target_include_directories(MULTI_FILES PUBLIC
#        ${WIZNET_DIR}/Ethernet
#        ${WIZNET_DIR}/Application/multicast
#)