cmake_minimum_required(VERSION 3.16)
project(Editor-Light-DataBase VERSION 1.0 LANGUAGES CXX)

if(DEFINED ENV{QTDIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)
endif()

find_package(Qt6 COMPONENTS Widgets Sql REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/Main.cpp
    src/Task.cpp
    src/Database.cpp
    src/Main_Window.cpp
    resources.qrc
)

set(HEADERS
    src/Task.h
    src/Database.h
    src/Main_Window.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets 
    Qt6::Sql
)