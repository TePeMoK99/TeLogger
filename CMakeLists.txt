cmake_minimum_required(VERSION 3.16)

project(TeLoggerLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)


option(BUILD_SHARED_LIBS "" ON)
option(BUILD_EXAMPLE "" OFF)
option(DEPLOY_LIB "" OFF)

add_library(TeLogger
    logger/telogger.h logger/telogger.cpp
    logger/telogger_global.h
)

target_link_libraries(TeLogger PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
)

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if (WIN32 AND build_type STREQUAL debug)
    set_target_properties(TeLogger PROPERTIES OUTPUT_NAME "TeLoggerd")
endif()

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

if (DEPLOY_LIB)
    include(deploy_library.cmake)
endif()
