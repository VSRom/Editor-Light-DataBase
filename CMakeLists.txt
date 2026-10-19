cmake_minimum_required(VERSION 3.30)
project(Editor-Light-DataBase VERSION 1.0 LANGUAGES CXX)

if(MSVC)
    add_compile_definitions(QT_NO_UNSAFE_ITERATORS)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{QTDIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 Sql REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/Main.cpp
    src/ConnectionController.cpp
    src/MainController.cpp
    src/StringListModel.cpp
    src/TableDataModel.cpp
    src/FilterModel.cpp
    src/TableRepository.cpp
    src/DatabaseWorker.cpp
    resources.qrc
    app.rc
)
set(HEADERS
    src/ConnectionController.h
    src/MainController.h
    src/StringListModel.h
    src/TableDataModel.h
    src/FilterModel.h
    src/TableRepository.h
    src/DatabaseWorker.h
)

target_include_directories(${PROJECT_NAME} PRIVATE core/vsc)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS}
    src/IConnection.h src/IConnection.cpp
    src/QtDbSession.h src/QtDbSession.cpp
    src/DbSessionFactory.h src/DbSessionFactory.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_UNSAFE_ITERATORS)

target_sources(Editor-Light-DataBase
  PRIVATE
    core/include/vsc/result.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Sql
)