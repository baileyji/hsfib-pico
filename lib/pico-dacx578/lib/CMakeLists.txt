add_library(pico-dacx578 INTERFACE)

target_include_directories(pico-dacx578
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pico-dacx578
  INTERFACE
  hardware_i2c
)

target_sources(pico-dacx578
  INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/dacx578.cpp
  ${CMAKE_CURRENT_LIST_DIR}/include/dacx578.h
)
