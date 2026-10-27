cmake_minimum_required(VERSION 3.20)
project(VSCQtAdapter LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

set(PUBLIC_HEADERS
    include/vsc/adapter/qt/qt_connection_factory.h
)
set(PRIVATE_HEADERS
    src/qt_connection.h
)
set(SOURCES
    src/qt_connection.cpp
    src/qt_connection_factory.cpp
)

add_library(vsc_qt_adapter STATIC ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})
add_library(vsc::qt_adapter ALIAS vsc_qt_adapter)

target_include_directories(vsc_qt_adapter
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(vsc_qt_adapter
    PUBLIC  vsc::core
    PRIVATE Qt6::Sql Qt6::Core
)

target_compile_features(vsc_qt_adapter PUBLIC cxx_std_23)