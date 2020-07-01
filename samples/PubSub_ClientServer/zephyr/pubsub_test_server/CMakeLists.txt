# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(pubsub_test_server)

add_definitions (-DWITH_STATIC_SECURITY_DATA=1 -DPUBSUB_STATIC_CONFIG=1 -DWITH_NANO_EXTENDED=1)
add_definitions (-DPUBLISH_PERIOD=1000 -DSUBSCRIBE_PERIOD=1000)

# add_definitions (-DDEBUG_PUBSUB_SCHEDULER_INFO=1)
# Add TEST include directory

SET(app_headers ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(app PRIVATE ${app_headers})

# =====================
# MBEDTLS SPECIFIC DEFS
# =====================

# custom config file
SET(mbedtls_config ${CMAKE_CURRENT_SOURCE_DIR}/src/tls_config)
target_include_directories(app PRIVATE ${tls_config})
zephyr_include_directories(${mbedtls_config})

FILE(GLOB tls_files ${CMAKE_CURRENT_SOURCE_DIR}/src/tls_config/*.c)
target_sources(app PRIVATE ${tls_files})

# ============================
# END OF MBEDTLS SPECIFIC DEFS
# ============================

# Add PUBSUB APPS includes directories

FILE(GLOB app_headers_pubsub_src_apps_pubsub_server ${CMAKE_CURRENT_SOURCE_DIR}/src/pubsub_server)

target_include_directories(app PRIVATE ${app_headers_pubsub_src_apps_pubsub_server})


# Add PUBSUB APPS sources directories

FILE(GLOB app_sources_pubsub_src_apps_pubsub_server_helpers_c ${CMAKE_CURRENT_SOURCE_DIR}/src/pubsub_server/helpers.c)

target_sources(app PRIVATE ${app_sources_pubsub_src_apps_pubsub_server_helpers_c})

FILE(GLOB app_sources_pubsub_src_apps_pubsub_server_pubsub_c ${CMAKE_CURRENT_SOURCE_DIR}/src/pubsub_server/pubsub.c)

target_sources(app PRIVATE ${app_sources_pubsub_src_apps_pubsub_server_pubsub_c})

FILE(GLOB app_sources_pubsub_src_apps_pubsub_server_server_c ${CMAKE_CURRENT_SOURCE_DIR}/src/pubsub_server/server.c)

target_sources(app PRIVATE ${app_sources_pubsub_src_apps_pubsub_server_server_c})


# Add TEST source directory

FILE(GLOB app_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)


target_sources(app PRIVATE ${app_sources})

# Add other path.
set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
