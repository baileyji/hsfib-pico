add_library(pico-pcal6416a INTERFACE)

target_include_directories(pico-pcal6416a
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pico-pcal6416a
  INTERFACE
  hardware_i2c
)

target_sources(pico-pcal6416a
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/pcal6416a.cpp
  ${CMAKE_CURRENT_LIST_DIR}/include/pcal6416a.h
)
