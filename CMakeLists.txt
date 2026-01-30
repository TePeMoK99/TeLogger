cmake_minimum_required(VERSION 3.16)

project(TeLoggerLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)


option(BUILD_EXAMPLE "" OFF)


add_library(TeLogger
    logger/telogger.h logger/telogger.cpp
    logger/telogger_global.h
)

target_link_libraries(TeLogger PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
)

add_compile_definitions(TeLogger TELOGGER_LIB PRIVATE)

target_include_directories(TeLogger PUBLIC ${CMAKE_SOURCE_DIR}/logger)

include(GNUInstallDirs)
install(TARGETS TeLogger
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (BUILD_EXAMPLE)
    add_subdirectory(Example)
endif()
