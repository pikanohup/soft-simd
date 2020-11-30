cmake_minimum_required(VERSION 3.10.2)

project(SoftSIMD)

set(softsimd_sources
cpp/softfloat.hpp
cpp/softfloat.cpp
)

# aux_source_directory(cpp DIR_SRCS)
# ARCH_ID x86-64 (TODO)

add_library(softsimd ${softsimd_sources})
set_property(TARGET softsimd PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET softsimd PROPERTY CXX_STANDARD 11)
set_property(TARGET softsimd PROPERTY CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
install(TARGETS softsimd
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


add_executable(baseline
  cpp/baseline.cpp)
target_link_libraries(baseline
  PRIVATE softsimd)
set_property(TARGET baseline PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET baseline PROPERTY CXX_STANDARD 11)
set_target_properties(baseline PROPERTIES LINK_FLAGS "-s WASM=1 -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1")
