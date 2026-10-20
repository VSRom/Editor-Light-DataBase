cmake_minimum_required(VERSION 3.20)
project(VSCCore LANGUAGES CXX)

add_library(vsc_core INTERFACE)
add_library(vsc::core ALIAS vsc_core)

target_include_directories(vsc_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_sources(vsc_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/column_metadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/connection_params.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/connection_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/i_connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/i_connection_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/i_transaction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/query.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/result_set.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/sql_error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vsc/value.h
)

target_compile_features(vsc_core INTERFACE cxx_std_23)
