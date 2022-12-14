cmake_minimum_required(VERSION 3.10)
project(web_server)

set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -Wno-unused-variable -Wpedantic -Wno-unknown-pragmas -Wno-sign-compare -Wwrite-strings -Wno-unused ")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDART_REQURED ON)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/server/include)
set(LIB_LOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src/lib_log)
set(LIB_SIG_HANDLER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src/lib_sig_handler)
set(LIB_HTTP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src/lib_http)
set(LIB_SET_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src/lib_set_config)

set(LIB_LOG_SRCS ${LIB_LOG_DIR}/logger.c)
set(LIB_SIG_HANDLER_SRCS ${LIB_SIG_HANDLER_DIR}/sig_handler.c)
set(LIB_HTTP_SRCS ${LIB_HTTP_DIR}/http.c
        ${LIB_HTTP_DIR}/read_req.c
        ${LIB_HTTP_DIR}/send_resp.c
        )
set(LIB_SET_CONFIG_SRCS ${LIB_SET_CONFIG_DIR}/set_config.c)
set(MAIN_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/server/cmd/main.c)

find_library(EV_LIBRARY
        NAMES libev ev evlib libev_static libev.so
        PATHS /usr/local/lib
        /usr/lib
        ${PROJECT_BINARY_DIR}/lib
        )

add_library(lib_log ${LIB_LOG_SRCS})
target_include_directories(lib_log PRIVATE ${INCLUDE_DIRS})

add_library(lib_sig_handler ${LIB_SIG_HANDLER_SRCS})
target_include_directories(lib_sig_handler PRIVATE ${INCLUDE_DIRS})
target_link_libraries(lib_sig_handler PRIVATE lib_log)

add_library(lib_http ${LIB_HTTP_SRCS})
target_include_directories(lib_http PRIVATE ${INCLUDE_DIRS})
target_link_libraries(lib_http PRIVATE lib_log)

add_library(lib_set_config ${LIB_SET_CONFIG_SRCS})
target_include_directories(lib_set_config PRIVATE ${INCLUDE_DIRS})

add_executable(web_server ${MAIN_SRCS})
target_include_directories(web_server PRIVATE ${INCLUDE_DIRS})
target_link_libraries(web_server PRIVATE lib_log)
target_link_libraries(web_server PRIVATE lib_http)
target_link_libraries(web_server PRIVATE lib_set_config)
target_link_libraries(web_server PRIVATE lib_sig_handler)
target_link_libraries(web_server PRIVATE ${EV_LIBRARY})
