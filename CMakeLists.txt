cmake_minimum_required(VERSION 3.16)
project(Editor-Light-DataBase VERSION 1.0 LANGUAGES CXX)

if(MSVC)
    add_compile_definitions(QT_NO_UNSAFE_ITERATORS)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{QTDIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

find_package(Qt6 COMPONENTS Widgets Sql REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/Main.cpp
    src/Database.cpp
    src/Main_Window.cpp
	src/Connection_Window.cpp
	src/Table_Explorer.cpp
    resources.qrc
 )

set(HEADERS
    src/Database.h
    src/Main_Window.h
	src/Connection_Window.h
	src/Table_Explorer.h
 )

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_UNSAFE_ITERATORS)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets 
    Qt6::Sql
)