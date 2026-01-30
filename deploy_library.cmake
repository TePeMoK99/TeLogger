file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/lib)

set(DEPLOY_DIR "${CMAKE_SOURCE_DIR}/lib/TeLogger")

file(MAKE_DIRECTORY
    ${DEPLOY_DIR}/lib
    ${DEPLOY_DIR}/include)

file(COPY
    ${CMAKE_SOURCE_DIR}/logger/telogger.h
    ${CMAKE_SOURCE_DIR}/logger/telogger_global.h
    DESTINATION ${DEPLOY_DIR}/include
)

file(GENERATE OUTPUT ${DEPLOY_DIR}/include/TeLogger
    CONTENT "#include \"telogger.h\""
)

set(LIB_NAME "libTeLogger")
if (WIN32)
    string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
    if (build_type STREQUAL debug)
        set(LIB_NAME "${LIB_NAME}d")
    endif()
    if(BUILD_SHARED_LIBS)
        set(LIB_NAME "${LIB_NAME}.dll")
    else()
        set(LIB_NAME "${LIB_NAME}.a")
    endif()
else()
    if(BUILD_SHARED_LIBS)
    set(LIB_NAME "${LIB_NAME}.so")
else()
    set(LIB_NAME "${LIB_NAME}.a")
endif()
endif()

message(${LIB_NAME})

file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_lib.cmake
    CONTENT
"file(COPY
    ${CMAKE_BINARY_DIR}/${LIB_NAME}
    DESTINATION ${DEPLOY_DIR}/lib
)
"
)

add_custom_command(TARGET TeLogger
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/copy_lib.cmake"
    WEBRATIM
)
