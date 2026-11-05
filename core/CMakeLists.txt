cmake_minimum_required(VERSION 3.20)
project(VSCCore LANGUAGES CXX)

add_library(vsc_core INTERFACE)
add_library(vsc::core ALIAS vsc_core)

file(GLOB_RECURSE VSC_CORE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

target_include_directories(vsc_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_sources(vsc_core INTERFACE ${VSC_CORE_HEADERS})

target_compile_features(vsc_core INTERFACE cxx_std_23)
